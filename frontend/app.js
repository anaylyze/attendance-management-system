/* =============================================
   app.js — AttendTrack Frontend Application
   Full state management, chart rendering, and
   interactive UI for attendance management system
   ============================================= */

/* ============================================================
   SAMPLE DATA (mirrors C++ CSV structure)
   ============================================================ */
let students = [
  { id: 1, name: "Priya Sharma",    section: "A", roll: "24CS001", year: 2024 },
  { id: 2, name: "Rahul Verma",     section: "A", roll: "24CS002", year: 2024 },
  { id: 3, name: "Ananya Singh",    section: "B", roll: "24CS003", year: 2024 },
  { id: 4, name: "Karan Mehta",     section: "B", roll: "24CS004", year: 2024 },
  { id: 5, name: "Divya Patel",     section: "A", roll: "24CS005", year: 2024 },
  { id: 6, name: "Arjun Nair",      section: "C", roll: "24CS006", year: 2024 },
  { id: 7, name: "Sneha Reddy",     section: "C", roll: "24CS007", year: 2024 },
  { id: 8, name: "Vikram Joshi",    section: "B", roll: "24CS008", year: 2024 },
  { id: 9, name: "Pooja Gupta",     section: "A", roll: "24CS009", year: 2024 },
  { id: 10, name: "Aman Kapoor",    section: "C", roll: "24CS010", year: 2024 },
  { id: 11, name: "Ritika Das",     section: "B", roll: "24CS011", year: 2024 },
  { id: 12, name: "Suresh Kumar",   section: "A", roll: "24CS012", year: 2024 },
];

// Records: { studentId, date, status, comment }
let attendanceRecords = [];

// Undo/Redo stacks
let undoStack = [];
let redoStack = [];

// Today's attendance (being edited)
let currentAttendance = {};

// Avatar colours (cycle through)
const AVATAR_COLORS = [
  "#10b981","#3b82f6","#f97316","#a855f7",
  "#06b6d4","#f43f5e","#84cc16","#f59e0b",
];

function avatarColor(id) { return AVATAR_COLORS[(id - 1) % AVATAR_COLORS.length]; }
function initials(name)  { return name.split(" ").map(w => w[0]).join("").slice(0,2).toUpperCase(); }

/* ============================================================
   ATTENDANCE HELPERS
   ============================================================ */
function getAttendance(studentId) {
  // Compute per-student overall attendance %
  const records = attendanceRecords.filter(r => r.studentId === studentId);
  if (records.length === 0) return { pct: null, present: 0, total: 0, absent: 0, late: 0, excused: 0 };
  const present  = records.filter(r => r.status === "PRESENT").length;
  const absent   = records.filter(r => r.status === "ABSENT").length;
  const late     = records.filter(r => r.status === "LATE").length;
  const excused  = records.filter(r => r.status === "EXCUSED").length;
  const effective = present + late * 0.5 + excused;
  return { pct: Math.round((effective / records.length) * 100), present, absent, late, excused, total: records.length };
}

function barColor(pct) {
  if (pct >= 85) return "#10b981";
  if (pct >= 75) return "#f97316";
  return "#ef4444";
}

function seedSampleAttendance() {
  // Seed 15 days of attendance so the dashboard has data
  const base = new Date(2026, 3, 1); // April 1
  const statuses = ["PRESENT","PRESENT","PRESENT","PRESENT","ABSENT","LATE","PRESENT","PRESENT","EXCUSED","PRESENT"];
  for (let d = 0; d < 15; d++) {
    const date = new Date(base);
    date.setDate(date.getDate() + d);
    // Skip weekends
    if (date.getDay() === 0 || date.getDay() === 6) continue;
    const dateStr = date.toISOString().split("T")[0];
    students.forEach((s, i) => {
      const varIdx = (i + d * 3) % statuses.length;
      attendanceRecords.push({ studentId: s.id, date: dateStr, status: statuses[varIdx], comment: "" });
    });
  }
}

/* ============================================================
   PAGE NAVIGATION
   ============================================================ */
function navigate(page) {
  document.querySelectorAll(".page").forEach(p => p.classList.remove("active"));
  document.querySelectorAll(".nav-item").forEach(n => n.classList.remove("active"));

  const pageEl = document.getElementById("page-" + page);
  const navEl  = document.getElementById("nav-" + page);
  if (pageEl) pageEl.classList.add("active");
  if (navEl)  navEl.classList.add("active");

  // Render page-specific content
  if (page === "dashboard")  renderDashboard();
  if (page === "students")   renderStudents();
  if (page === "attendance") renderAttendancePage();
  if (page === "analytics")  renderAnalytics();

  // Close sidebar on mobile
  if (window.innerWidth <= 768) {
    document.getElementById("sidebar").classList.remove("open");
  }
}

/* ============================================================
   TOAST
   ============================================================ */
let toastTimer;
function showToast(msg, type = "success") {
  const t = document.getElementById("toast");
  t.textContent = msg;
  t.className = "toast " + type + " show";
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => t.classList.remove("show"), 3000);
}

/* ============================================================
   DASHBOARD
   ============================================================ */
function renderDashboard() {
  renderTodayStats();
  renderWeeklyChart();
  renderDonut();
  renderAlerts();
  renderActivity();
  document.getElementById("todayDate").textContent = new Date().toLocaleDateString("en-IN", { weekday:"short", day:"numeric", month:"short" });
}

function getTodayDateStr() {
  return new Date().toISOString().split("T")[0];
}

function renderTodayStats() {
  const today = getTodayDateStr();
  const todayRecs = attendanceRecords.filter(r => r.date === today);
  const present = todayRecs.filter(r => r.status === "PRESENT").length;
  const absent  = todayRecs.filter(r => r.status === "ABSENT").length;
  const late    = todayRecs.filter(r => r.status === "LATE").length;
  const total   = students.length;
  const pct     = total ? Math.round((present / total) * 100) : 0;
  const low     = students.filter(s => { const a = getAttendance(s.id); return a.pct !== null && a.pct < 75; }).length;

  document.getElementById("totalStudents").textContent = total;
  document.getElementById("presentToday").textContent  = present || (todayRecs.length === 0 ? "—" : present);
  document.getElementById("lowAttendance").textContent = low;
  document.getElementById("snap-present").textContent  = present;
  document.getElementById("snap-absent").textContent   = absent;
  document.getElementById("snap-late").textContent     = late;
  document.getElementById("donutPct").textContent      = (todayRecs.length ? pct + "%" : "—");
}

/* ---- Simple Canvas Bar Chart ---- */
function renderWeeklyChart() {
  const canvas = document.getElementById("weeklyChart");
  if (!canvas) return;
  const ctx = canvas.getContext("2d");
  const W = canvas.parentElement.offsetWidth;
  const H = 220;
  canvas.width = W; canvas.height = H;

  const days = ["Mon","Tue","Wed","Thu","Fri"];
  const today = new Date(2026, 3, 19); // April 19
  const dayData = days.map((_, i) => {
    const d = new Date(today);
    d.setDate(d.getDate() - (4 - i));
    const ds = d.toISOString().split("T")[0];
    const recs = attendanceRecords.filter(r => r.date === ds);
    return {
      present: recs.filter(r => r.status === "PRESENT").length,
      absent:  recs.filter(r => r.status === "ABSENT").length,
      late:    recs.filter(r => r.status === "LATE").length,
    };
  });

  const maxTotal = Math.max(...dayData.map(d => d.present + d.absent + d.late), 1);
  const padL = 30, padR = 20, padT = 10, padB = 30;
  const chartH = H - padT - padB;
  const groupW = (W - padL - padR) / days.length;
  const barW   = groupW * 0.55;

  ctx.clearRect(0, 0, W, H);

  // Grid lines
  for (let i = 0; i <= 4; i++) {
    const y = padT + (chartH / 4) * i;
    ctx.strokeStyle = "#e2e8f0";
    ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(padL, y); ctx.lineTo(W - padR, y); ctx.stroke();
  }

  dayData.forEach((d, i) => {
    const x = padL + i * groupW + groupW / 2;
    const sections = [
      { val: d.late,    color: "#fb923c" },
      { val: d.absent,  color: "#f87171" },
      { val: d.present, color: "#10b981" },
    ];
    let stackY = padT + chartH;
    sections.forEach(s => {
      const bh = (s.val / maxTotal) * chartH;
      if (bh < 1) return;
      stackY -= bh;
      ctx.fillStyle = s.color;
      roundRect(ctx, x - barW / 2, stackY, barW, bh, 5);
      ctx.fill();
    });

    // Day label
    ctx.fillStyle = "#94a3b8";
    ctx.font = "600 11px 'DM Sans', sans-serif";
    ctx.textAlign = "center";
    ctx.fillText(days[i], x, H - 8);
  });
}

function roundRect(ctx, x, y, w, h, r) {
  if (h < r * 2) r = h / 2;
  ctx.beginPath();
  ctx.moveTo(x + r, y);
  ctx.lineTo(x + w - r, y);
  ctx.arcTo(x + w, y, x + w, y + h, r);
  ctx.lineTo(x + w, y + h);
  ctx.lineTo(x, y + h);
  ctx.arcTo(x, y, x + r, y, r);
  ctx.closePath();
}

/* ---- Donut Chart ---- */
function renderDonut() {
  const canvas = document.getElementById("donutChart");
  if (!canvas) return;
  const ctx = canvas.getContext("2d");
  const W = 160, H = 160, cx = W/2, cy = H/2, r = 60, ir = 40;
  canvas.width = W; canvas.height = H;

  const today = getTodayDateStr();
  const recs  = attendanceRecords.filter(r => r.date === today);
  const present = recs.filter(r => r.status === "PRESENT").length;
  const absent  = recs.filter(r => r.status === "ABSENT").length;
  const late    = recs.filter(r => r.status === "LATE").length;
  const excused = recs.filter(r => r.status === "EXCUSED").length;
  const total   = present + absent + late + excused;

  const segments = total > 0
    ? [
        { val: present, color: "#10b981" },
        { val: absent,  color: "#f87171" },
        { val: late,    color: "#fb923c" },
        { val: excused, color: "#3b82f6" },
      ]
    : [{ val: 1, color: "#e2e8f0" }];

  const totalVal = segments.reduce((a, s) => a + s.val, 0);
  let angle = -Math.PI / 2;
  ctx.clearRect(0, 0, W, H);

  segments.forEach(s => {
    const sweep = (s.val / totalVal) * 2 * Math.PI;
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.arc(cx, cy, r, angle, angle + sweep);
    ctx.closePath();
    ctx.fillStyle = s.color;
    ctx.fill();
    angle += sweep;
  });

  // Inner hole
  ctx.beginPath();
  ctx.arc(cx, cy, ir, 0, 2 * Math.PI);
  ctx.fillStyle = "#fff";
  ctx.fill();
}

/* ---- Alerts ---- */
function renderAlerts() {
  const list = document.getElementById("alertList");
  if (!list) return;
  const lowStudents = students.filter(s => {
    const a = getAttendance(s.id);
    return a.pct !== null && a.pct < 75;
  }).sort((a, b) => getAttendance(a.id).pct - getAttendance(b.id).pct);

  if (lowStudents.length === 0) {
    list.innerHTML = `<div style="text-align:center;color:var(--text-secondary);padding:20px;font-size:.88rem;">
      🎉 All students have good attendance!
    </div>`;
    return;
  }

  list.innerHTML = lowStudents.map(s => {
    const a = getAttendance(s.id);
    return `<div class="alert-item">
      <div class="alert-avatar" style="background:${avatarColor(s.id)}">${initials(s.name)}</div>
      <div class="alert-info">
        <div class="alert-name">${s.name}</div>
        <div class="alert-pct">${a.pct}% attendance · ${a.total} days tracked</div>
      </div>
      <span class="alert-badge">⚠ ${a.pct}%</span>
    </div>`;
  }).join("");
}

/* ---- Activity ---- */
function renderActivity() {
  const el = document.getElementById("activityList");
  if (!el) return;
  const activities = [
    { icon: "✅", text: "Marked attendance for Section A — 12/12 present", time: "Today, 9:15 AM" },
    { icon: "🎓", text: "Added new student: Sneha Reddy (Section C)", time: "Yesterday, 3:42 PM" },
    { icon: "📊", text: "Analytics report generated", time: "Yesterday, 2:00 PM" },
    { icon: "✏️", text: "Edited attendance for Karan Mehta (Apr 17)", time: "2 days ago" },
    { icon: "⚠️", text: "Low attendance alert: 3 students below 75%", time: "3 days ago" },
  ];
  el.innerHTML = activities.map(a => `
    <div class="activity-item">
      <span class="activity-icon">${a.icon}</span>
      <div>
        <div class="activity-text">${a.text}</div>
        <div class="activity-time">${a.time}</div>
      </div>
    </div>
  `).join("");
}

/* ============================================================
   STUDENTS PAGE
   ============================================================ */
function renderStudents(filter = "", section = "", sortBy = "name") {
  const grid = document.getElementById("studentGrid");
  if (!grid) return;

  let list = [...students];
  if (filter)  list = list.filter(s => s.name.toLowerCase().includes(filter.toLowerCase()) || s.section.toLowerCase() === filter.toLowerCase());
  if (section) list = list.filter(s => s.section === section);
  if (sortBy === "name")       list.sort((a, b) => a.name.localeCompare(b.name));
  else if (sortBy === "id")    list.sort((a, b) => a.id - b.id);
  else if (sortBy === "attendance") {
    list.sort((a, b) => {
      const pa = getAttendance(a.id).pct ?? 100;
      const pb = getAttendance(b.id).pct ?? 100;
      return pa - pb;
    });
  }

  if (list.length === 0) {
    grid.innerHTML = `<div style="grid-column:1/-1;text-align:center;padding:40px;color:var(--text-secondary);">No students found.</div>`;
    return;
  }

  grid.innerHTML = list.map(s => {
    const att = getAttendance(s.id);
    const pct = att.pct ?? null;
    const bc  = pct !== null ? barColor(pct) : "#94a3b8";
    return `
    <div class="student-card" id="sc-${s.id}">
      <div class="student-card-header">
        <div class="student-avatar" style="background:${avatarColor(s.id)}">${initials(s.name)}</div>
        <div>
          <div class="student-name">${s.name}</div>
          <div class="student-meta">Roll: ${s.roll} · ${s.year}</div>
        </div>
      </div>
      <div class="student-stats">
        <span class="student-stat-chip">Section ${s.section}</span>
        ${pct !== null ? `<span class="student-stat-chip" style="background:${bc}22;color:${bc}">${pct}% attendance</span>` : `<span class="student-stat-chip">No data</span>`}
      </div>
      ${pct !== null ? `
      <div class="attendance-bar-wrap">
        <div class="attendance-bar-label">
          <span>Attendance</span><span style="color:${bc}">${pct}%</span>
        </div>
        <div class="attendance-bar">
          <div class="attendance-bar-fill" style="width:${pct}%;background:${bc};"></div>
        </div>
      </div>` : ""}
      <div class="student-card-actions">
        <button class="student-action-btn" onclick="goToStudentAttendance(${s.id})">📋 Attendance</button>
        <button class="student-action-btn danger" onclick="removeStudent(${s.id})">🗑 Remove</button>
      </div>
    </div>`;
  }).join("");
}

function goToStudentAttendance(id) {
  navigate("attendance");
  showToast(`Viewing attendance page`, "info");
}

function removeStudent(id) {
  const s = students.find(st => st.id === id);
  if (!s) return;
  if (!confirm(`Remove ${s.name} from the system? This cannot be undone.`)) return;
  students = students.filter(st => st.id !== id);
  attendanceRecords = attendanceRecords.filter(r => r.studentId !== id);
  renderStudents(
    document.getElementById("studentSearch")?.value || "",
    document.getElementById("sectionFilter")?.value || "",
    document.getElementById("sortStudents")?.value || "name"
  );
  showToast(`${s.name} removed`, "error");
}

/* ============================================================
   ATTENDANCE PAGE
   ============================================================ */
function renderAttendancePage() {
  // Init date to today
  const dp = document.getElementById("attendanceDate");
  if (dp && !dp.value) dp.value = getTodayDateStr();
  loadAttendanceForDate(dp?.value || getTodayDateStr());
}

function loadAttendanceForDate(dateStr) {
  currentAttendance = {};
  const existing = attendanceRecords.filter(r => r.date === dateStr);
  existing.forEach(r => {
    currentAttendance[r.studentId] = { status: r.status, comment: r.comment || "" };
  });
  if (Object.keys(currentAttendance).length === 0) {
    students.forEach(s => { currentAttendance[s.id] = { status: "", comment: "" }; });
  }
  renderAttendanceTable(dateStr);
}

function renderAttendanceTable(dateStr) {
  const body = document.getElementById("attendanceBody");
  if (!body) return;

  body.innerHTML = students.map(s => {
    const rec = currentAttendance[s.id] || { status: "", comment: "" };
    const statuses = ["PRESENT","ABSENT","LATE","EXCUSED"];
    const labels   = { PRESENT:"✅ Present", ABSENT:"❌ Absent", LATE:"🕐 Late", EXCUSED:"💙 Excused" };
    const btns = statuses.map(st =>
      `<button class="status-btn ${rec.status === st ? 'active' : ''}"
         data-status="${st}" data-sid="${s.id}">${labels[st]}</button>`
    ).join("");
    return `
    <tr>
      <td>
        <div style="display:flex;align-items:center">
          <div class="table-avatar" style="background:${avatarColor(s.id)}">${initials(s.name)}</div>
          <span style="font-weight:600">${s.name}</span>
        </div>
      </td>
      <td style="color:var(--text-secondary);font-size:.82rem">${s.roll}</td>
      <td><span class="badge">Sec ${s.section}</span></td>
      <td><div class="status-toggle">${btns}</div></td>
      <td><input class="comment-input" type="text" placeholder="Add comment…"
           value="${rec.comment || ''}" data-sid="${s.id}" /></td>
    </tr>`;
  }).join("");

  // Bind status button clicks
  body.querySelectorAll(".status-btn").forEach(btn => {
    btn.addEventListener("click", () => {
      const sid    = parseInt(btn.dataset.sid);
      const status = btn.dataset.status;
      const prev   = currentAttendance[sid]?.status || "";
      
      // Push undo
      undoStack.push({ type: "statusChange", sid, prev, next: status, date: dateStr });
      redoStack = [];
      
      currentAttendance[sid] = { ...currentAttendance[sid], status };
      // Update buttons in same row
      btn.closest(".status-toggle").querySelectorAll(".status-btn").forEach(b => {
        b.classList.toggle("active", b.dataset.status === status);
      });
    });
  });

  // Bind comment inputs
  body.querySelectorAll(".comment-input").forEach(inp => {
    inp.addEventListener("input", () => {
      const sid = parseInt(inp.dataset.sid);
      currentAttendance[sid] = { ...currentAttendance[sid], comment: inp.value };
    });
  });
}

function saveAttendance() {
  const dateStr = document.getElementById("attendanceDate")?.value;
  if (!dateStr) { showToast("Please select a date", "error"); return; }

  // Remove existing records for this date
  attendanceRecords = attendanceRecords.filter(r => r.date !== dateStr);

  // Add current
  Object.entries(currentAttendance).forEach(([sid, rec]) => {
    if (rec.status) {
      attendanceRecords.push({ studentId: parseInt(sid), date: dateStr, status: rec.status, comment: rec.comment });
    }
  });

  showToast("✅ Attendance saved successfully!", "success");
}

function markAll(status) {
  const dateStr = document.getElementById("attendanceDate")?.value || getTodayDateStr();
  students.forEach(s => {
    currentAttendance[s.id] = { ...currentAttendance[s.id], status };
  });
  renderAttendanceTable(dateStr);
  showToast(`All students marked as ${status}`, "info");
}

function undo() {
  if (undoStack.length === 0) { showToast("Nothing to undo", "error"); return; }
  const cmd = undoStack.pop();
  redoStack.push(cmd);
  if (cmd.type === "statusChange") {
    currentAttendance[cmd.sid] = { ...currentAttendance[cmd.sid], status: cmd.prev };
    renderAttendanceTable(cmd.date);
  }
  showToast("↩ Undone", "info");
}

function redo() {
  if (redoStack.length === 0) { showToast("Nothing to redo", "error"); return; }
  const cmd = redoStack.pop();
  undoStack.push(cmd);
  if (cmd.type === "statusChange") {
    currentAttendance[cmd.sid] = { ...currentAttendance[cmd.sid], status: cmd.next };
    renderAttendanceTable(cmd.date);
  }
  showToast("↪ Redone", "info");
}

/* ============================================================
   ANALYTICS PAGE
   ============================================================ */
function renderAnalytics() {
  // Per-student list, sorted worst first
  const studentData = students.map(s => ({ s, a: getAttendance(s.id) }))
    .filter(x => x.a.pct !== null)
    .sort((a, b) => a.a.pct - b.a.pct);

  const list = document.getElementById("analyticsStudentList");
  if (list) {
    list.innerHTML = studentData.map(({ s, a }, i) => {
      const bc = barColor(a.pct);
      const isDanger = a.pct < 75;
      return `
      <div class="analytics-student-row">
        <div class="analytics-rank ${isDanger ? 'danger' : ''}">${i + 1}</div>
        <div style="display:flex;align-items:center;gap:10px;flex:0 0 180px">
          <div class="table-avatar" style="background:${avatarColor(s.id)};width:32px;height:32px;font-size:.76rem">${initials(s.name)}</div>
          <div>
            <div class="analytics-student-name">${s.name}</div>
            <div class="analytics-student-section">Section ${s.section} · ${a.total} days</div>
          </div>
        </div>
        <div class="analytics-bar-wrap">
          <div class="analytics-bar"><div class="analytics-bar-fill" style="width:${a.pct}%;background:${bc}"></div></div>
        </div>
        <div class="analytics-pct" style="color:${bc}">${a.pct}%</div>
      </div>`;
    }).join("");
  }

  // Section comparison
  const sections = ["A", "B", "C"];
  const sectionBarsEl = document.getElementById("sectionBars");
  if (sectionBarsEl) {
    sectionBarsEl.innerHTML = sections.map(sec => {
      const secStudents = students.filter(s => s.section === sec);
      const pcts = secStudents.map(s => getAttendance(s.id).pct).filter(p => p !== null);
      const avg  = pcts.length ? Math.round(pcts.reduce((a, b) => a + b, 0) / pcts.length) : 0;
      return `
      <div class="section-bar-row">
        <div class="section-name">Section ${sec}</div>
        <div class="section-bar"><div class="section-bar-fill" style="width:${avg}%"></div></div>
        <div class="section-pct">${avg}%</div>
      </div>`;
    }).join("");
  }
}

/* ============================================================
   REPORTS PAGE
   ============================================================ */
function generateReport(type) {
  let content = "";
  const now = new Date().toLocaleString("en-IN");

  if (type === "analytics") {
    content = `ANALYTICS REPORT
Generated: ${now}
${"=".repeat(60)}

STUDENT ATTENDANCE RANKING (Worst → Best)
${"─".repeat(60)}

`;
    const sorted = students
      .map(s => ({ s, a: getAttendance(s.id) }))
      .filter(x => x.a.pct !== null)
      .sort((a, b) => a.a.pct - b.a.pct);

    sorted.forEach(({ s, a }, i) => {
      const bar = "█".repeat(Math.round(a.pct / 5)) + "░".repeat(20 - Math.round(a.pct / 5));
      content += `${String(i+1).padStart(2)}. ${s.name.padEnd(20)} [${bar}] ${a.pct}%\n`;
      content += `    Present: ${a.present}  Absent: ${a.absent}  Late: ${a.late}  Total: ${a.total}\n\n`;
    });

  } else if (type === "low") {
    content = `LOW ATTENDANCE REPORT
Generated: ${now}
${"=".repeat(60)}

STUDENTS BELOW 75% THRESHOLD
${"─".repeat(60)}

`;
    const low = students
      .map(s => ({ s, a: getAttendance(s.id) }))
      .filter(x => x.a.pct !== null && x.a.pct < 75)
      .sort((a, b) => a.a.pct - b.a.pct);

    if (low.length === 0) {
      content += "✓ No students below the 75% threshold. Great job!\n";
    } else {
      low.forEach(({ s, a }) => {
        content += `⚠  ${s.name} (${s.roll}) — Section ${s.section}\n`;
        content += `   Attendance: ${a.pct}%  |  Present: ${a.present}/${a.total} days\n\n`;
      });
    }

  } else if (type === "summary") {
    content = `MONTHLY SUMMARY REPORT — April 2026
Generated: ${now}
${"=".repeat(60)}

SECTION OVERVIEW
${"─".repeat(60)}
`;
    ["A","B","C"].forEach(sec => {
      const ss = students.filter(s => s.section === sec);
      const pcts = ss.map(s => getAttendance(s.id).pct).filter(p => p !== null);
      const avg  = pcts.length ? Math.round(pcts.reduce((a, b) => a + b, 0) / pcts.length) : "N/A";
      content += `Section ${sec}: ${ss.length} students, avg attendance: ${avg}%\n`;
    });
    content += `\nTotal Students: ${students.length}\n`;
    content += `Total Records:  ${attendanceRecords.length}\n`;

  } else if (type === "csv") {
    content = "studentId,name,section,roll,date,status,comment\n";
    attendanceRecords.forEach(r => {
      const s = students.find(st => st.id === r.studentId);
      if (s) content += `${s.id},${s.name},${s.section},${s.roll},${r.date},${r.status},"${r.comment}"\n`;
    });
  }

  document.getElementById("reportPreview").textContent = content;
  document.getElementById("reportPreviewCard").style.display = "block";
  document.getElementById("reportPreviewCard").scrollIntoView({ behavior: "smooth" });

  // Store for download
  window._reportContent = content;
  window._reportType    = type;
}

function downloadReport() {
  if (!window._reportContent) return;
  const ext  = window._reportType === "csv" ? "csv" : "txt";
  const blob = new Blob([window._reportContent], { type: "text/plain" });
  const url  = URL.createObjectURL(blob);
  const a    = document.createElement("a");
  a.href = url;
  a.download = `${window._reportType}_report.${ext}`;
  a.click();
  URL.revokeObjectURL(url);
  showToast("📥 Report downloaded!", "success");
}

/* ============================================================
   MODAL HELPERS
   ============================================================ */
function openModal(id)  { document.getElementById(id).classList.add("open"); }
function closeModal(id) { document.getElementById(id).classList.remove("open"); }

/* ============================================================
   ADD STUDENT
   ============================================================ */
function addStudent(name, section, roll, year) {
  const newId = students.length > 0 ? Math.max(...students.map(s => s.id)) + 1 : 1;
  students.push({ id: newId, name, section, roll, year: parseInt(year) });
  showToast(`🎉 ${name} added successfully!`, "success");
  renderStudents();
}

/* ============================================================
   EVENT LISTENERS
   ============================================================ */
document.addEventListener("DOMContentLoaded", () => {
  seedSampleAttendance();
  renderDashboard();

  // Navigation
  document.querySelectorAll(".nav-item").forEach(item => {
    item.addEventListener("click", e => {
      e.preventDefault();
      navigate(item.dataset.page);
    });
  });

  // Hamburger
  document.getElementById("hamburger")?.addEventListener("click", () => {
    document.getElementById("sidebar").classList.toggle("open");
  });

  // Dashboard quick mark
  document.getElementById("quickMarkBtn")?.addEventListener("click", () => openModal("quickMarkModal"));
  document.getElementById("closeQuickModal")?.addEventListener("click", () => closeModal("quickMarkModal"));
  document.getElementById("qmPresent")?.addEventListener("click", () => {
    const today = getTodayDateStr();
    students.forEach(s => {
      attendanceRecords = attendanceRecords.filter(r => !(r.studentId === s.id && r.date === today));
      attendanceRecords.push({ studentId: s.id, date: today, status: "PRESENT", comment: "" });
    });
    closeModal("quickMarkModal");
    renderDashboard();
    showToast("✅ All students marked Present!", "success");
  });
  document.getElementById("qmAbsent")?.addEventListener("click", () => {
    const today = getTodayDateStr();
    students.forEach(s => {
      attendanceRecords = attendanceRecords.filter(r => !(r.studentId === s.id && r.date === today));
      attendanceRecords.push({ studentId: s.id, date: today, status: "ABSENT", comment: "" });
    });
    closeModal("quickMarkModal");
    renderDashboard();
    showToast("Attendance marked Absent for all", "info");
  });

  // Students page
  document.getElementById("addStudentBtn")?.addEventListener("click", () => openModal("addStudentModal"));
  document.getElementById("closeAddStudentModal")?.addEventListener("click", () => closeModal("addStudentModal"));
  document.getElementById("cancelAddStudent")?.addEventListener("click", () => closeModal("addStudentModal"));
  document.getElementById("addStudentForm")?.addEventListener("submit", e => {
    e.preventDefault();
    const name    = document.getElementById("sName").value.trim();
    const section = document.getElementById("sSection").value;
    const roll    = document.getElementById("sRoll").value.trim();
    const year    = document.getElementById("sYear").value;
    if (!name || !section || !roll) return;
    addStudent(name, section, roll, year);
    closeModal("addStudentModal");
    document.getElementById("addStudentForm").reset();
  });

  // Search / filter
  document.getElementById("studentSearch")?.addEventListener("input", e => {
    renderStudents(e.target.value, document.getElementById("sectionFilter").value, document.getElementById("sortStudents").value);
  });
  document.getElementById("sectionFilter")?.addEventListener("change", e => {
    renderStudents(document.getElementById("studentSearch").value, e.target.value, document.getElementById("sortStudents").value);
  });
  document.getElementById("sortStudents")?.addEventListener("change", e => {
    renderStudents(document.getElementById("studentSearch").value, document.getElementById("sectionFilter").value, e.target.value);
  });

  // Attendance
  document.getElementById("loadDateBtn")?.addEventListener("click", () => {
    const d = document.getElementById("attendanceDate")?.value;
    if (d) loadAttendanceForDate(d);
    else showToast("Please select a date", "error");
  });
  document.getElementById("markAllPresent")?.addEventListener("click", () => markAll("PRESENT"));
  document.getElementById("markAllAbsent")?.addEventListener("click",  () => markAll("ABSENT"));
  document.getElementById("saveAttendanceBtn")?.addEventListener("click", saveAttendance);
  document.getElementById("undoBtn")?.addEventListener("click", undo);
  document.getElementById("redoBtn")?.addEventListener("click", redo);

  // Reports
  document.querySelectorAll(".report-btn").forEach(btn => {
    btn.addEventListener("click", () => generateReport(btn.dataset.report));
  });
  document.getElementById("downloadReportBtn")?.addEventListener("click", downloadReport);

  // Close modals on overlay click
  document.querySelectorAll(".modal-overlay").forEach(m => {
    m.addEventListener("click", e => { if (e.target === m) m.classList.remove("open"); });
  });

  // Resize re-renders chart
  window.addEventListener("resize", () => {
    const activePage = document.querySelector(".page.active");
    if (activePage?.id === "page-dashboard") renderWeeklyChart();
  });
});
