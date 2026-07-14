/*
 * app.js — browser front-end. It does NOT implement any pathfinding: it draws
 * the grid, and delegates every search to the C++ engine compiled to
 * WebAssembly (Module.solve). The returned visited-order + path are animated.
 */

const ROWS = 25;
const COLS = 51;
const START = { row: 12, col: 8 };
const FINISH = { row: 12, col: 42 };

let engine = null;              // the WebAssembly module (C++ solve())
let grid = [];
let isMouseDown = false;
let isRunning = false;
let dragging = null;            // 'start' | 'finish' | null

const boardEl = document.getElementById('board');
const algoSel = document.getElementById('algo');
const speedSel = document.getElementById('speed');
const statVisited = document.getElementById('stat-visited');
const statPath = document.getElementById('stat-path');
const statStatus = document.getElementById('stat-status');

/* --------------------- load the C++ / WebAssembly engine ----------------- */
PathfinderModule().then((mod) => {
  engine = mod;
  document.getElementById('engine').classList.add('ready');
  document.getElementById('engine-text').textContent = 'C++ engine ready (WASM)';
  statStatus.textContent = 'Ready';
  setControls(true);
});

/* --------------------------- grid construction --------------------------- */
function createNode(row, col) {
  return {
    row, col,
    isStart: row === START.row && col === START.col,
    isFinish: row === FINISH.row && col === FINISH.col,
    isWall: false,
  };
}

function buildGrid() {
  grid = [];
  boardEl.innerHTML = '';
  boardEl.style.gridTemplateColumns = `repeat(${COLS}, 1fr)`;
  for (let r = 0; r < ROWS; r++) {
    const rowArr = [];
    for (let c = 0; c < COLS; c++) {
      const node = createNode(r, c);
      rowArr.push(node);
      const cell = document.createElement('div');
      cell.className = 'cell';
      cell.id = `c-${r}-${c}`;
      if (node.isStart) cell.classList.add('start');
      if (node.isFinish) cell.classList.add('finish');
      attachMouse(cell, node);
      boardEl.appendChild(cell);
    }
    grid.push(rowArr);
  }
}
function cellEl(node) { return document.getElementById(`c-${node.row}-${node.col}`); }

/* ---------------------------- wall drawing ------------------------------- */
function attachMouse(cell, node) {
  cell.addEventListener('mousedown', (e) => {
    e.preventDefault();
    if (isRunning) return;
    isMouseDown = true;
    if (node.isStart) { dragging = 'start'; return; }
    if (node.isFinish) { dragging = 'finish'; return; }
    toggleWall(node);
  });
  cell.addEventListener('mouseenter', () => {
    if (!isMouseDown || isRunning) return;
    if (dragging === 'start') { moveEndpoint('start', node); return; }
    if (dragging === 'finish') { moveEndpoint('finish', node); return; }
    if (!node.isStart && !node.isFinish) toggleWall(node, true);
  });
}
document.addEventListener('mouseup', () => { isMouseDown = false; dragging = null; });

function toggleWall(node, drawOnly = false) {
  if (node.isStart || node.isFinish) return;
  node.isWall = drawOnly ? true : !node.isWall;
  cellEl(node).classList.toggle('wall', node.isWall);
}
function moveEndpoint(kind, node) {
  if (node.isWall) return;
  const flag = kind === 'start' ? 'isStart' : 'isFinish';
  for (const row of grid) for (const n of row) {
    if (n[flag]) { n[flag] = false; cellEl(n).classList.remove(kind); }
  }
  node[flag] = true;
  cellEl(node).classList.add(kind);
}
function findNode(flag) {
  for (const row of grid) for (const n of row) if (n[flag]) return n;
  return null;
}

/* ------------------------------ resets ----------------------------------- */
function clearSearchStyling() {
  for (const row of grid) for (const n of row) cellEl(n).classList.remove('visited', 'path');
}
function clearWalls() {
  if (isRunning) return;
  for (const row of grid) for (const n of row) {
    if (n.isWall) { n.isWall = false; cellEl(n).classList.remove('wall'); }
  }
  clearSearchStyling(); resetStats();
}
function resetBoard() { if (!isRunning) { buildGrid(); resetStats(); } }
function resetStats() {
  statVisited.textContent = '—'; statPath.textContent = '—';
  statStatus.textContent = 'Ready'; statStatus.className = '';
}
function randomWalls() {
  if (isRunning) return;
  clearWalls();
  for (const row of grid) for (const n of row) {
    if (!n.isStart && !n.isFinish && Math.random() < 0.20) {
      n.isWall = true; cellEl(n).classList.add('wall');
    }
  }
}

/* ---------------------------- visualization ------------------------------ */
const SPEEDS = { fast: 6, medium: 18, slow: 42 };
function sleep(ms) { return new Promise(res => setTimeout(res, ms)); }

// Pack the wall layout into the row-major '0'/'1' string the C++ engine expects.
function wallString() {
  let s = '';
  for (let r = 0; r < ROWS; r++)
    for (let c = 0; c < COLS; c++)
      s += grid[r][c].isWall ? '1' : '0';
  return s;
}

async function visualize() {
  if (isRunning || !engine) return;
  clearSearchStyling();
  isRunning = true; setControls(false);
  statStatus.textContent = 'Searching…'; statStatus.className = '';

  const start = findNode('isStart');
  const finish = findNode('isFinish');

  // ---- call into the C++ / WebAssembly engine ----
  const json = engine.solve(wallString(), ROWS, COLS,
                            start.row, start.col, finish.row, finish.col, algoSel.value);
  const res = JSON.parse(json);   // { found, order: [[r,c]...], path: [[r,c]...] }

  const delay = SPEEDS[speedSel.value];
  for (let i = 0; i < res.order.length; i++) {
    const [r, c] = res.order[i];
    const node = grid[r][c];
    if (!node.isStart && !node.isFinish) cellEl(node).classList.add('visited');
    if (delay) await sleep(delay);
  }
  statVisited.textContent = res.order.length;

  if (res.found) {
    for (const [r, c] of res.path) {
      const node = grid[r][c];
      if (!node.isStart && !node.isFinish) cellEl(node).classList.add('path');
      await sleep(28);
    }
    statPath.textContent = res.path.length - 1;
    statStatus.textContent = 'Path found'; statStatus.className = 'ok';
  } else {
    statPath.textContent = '—';
    statStatus.textContent = 'No path (blocked)'; statStatus.className = 'bad';
  }

  isRunning = false; setControls(true);
}

function setControls(enabled) {
  document.querySelectorAll('button, select').forEach(el => { el.disabled = !enabled; });
}

/* ------------------------------- wiring ---------------------------------- */
document.getElementById('run').addEventListener('click', visualize);
document.getElementById('clear-walls').addEventListener('click', clearWalls);
document.getElementById('reset').addEventListener('click', resetBoard);
document.getElementById('maze').addEventListener('click', randomWalls);

buildGrid();
