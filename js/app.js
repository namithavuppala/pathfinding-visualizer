/*
 * app.js — grid model, rendering, user interaction, and animation.
 * The search algorithms themselves live in algorithms.js.
 */

const ROWS = 25;
const COLS = 51;
const START = { row: 12, col: 8 };
const FINISH = { row: 12, col: 42 };

let grid = [];
let isMouseDown = false;
let isRunning = false;
let dragging = null; // 'start' | 'finish' | null

const boardEl = document.getElementById('board');
const algoSel = document.getElementById('algo');
const speedSel = document.getElementById('speed');
const statVisited = document.getElementById('stat-visited');
const statPath = document.getElementById('stat-path');
const statStatus = document.getElementById('stat-status');

/* --------------------------- grid construction --------------------------- */
function createNode(row, col) {
  return {
    row, col,
    isStart: row === START.row && col === START.col,
    isFinish: row === FINISH.row && col === FINISH.col,
    isWall: false,
    // search bookkeeping (reset before each run)
    distance: Infinity,
    heuristic: 0,
    visited: false,
    previousNode: null,
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
  cell.addEventListener('mouseup', () => { isMouseDown = false; dragging = null; });
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
  const cls = kind;
  // clear previous
  for (const row of grid) for (const n of row) {
    if (n[flag]) { n[flag] = false; cellEl(n).classList.remove(cls); }
  }
  node[flag] = true;
  cellEl(node).classList.add(cls);
}

/* ------------------------------ resets ----------------------------------- */
function findNode(flag) {
  for (const row of grid) for (const n of row) if (n[flag]) return n;
  return null;
}

// Clear search state + visited/path styling, keep walls and endpoints.
function softReset() {
  for (const row of grid) {
    for (const n of row) {
      n.distance = Infinity;
      n.heuristic = 0;
      n.visited = false;
      n.previousNode = null;
      const el = cellEl(n);
      el.classList.remove('visited', 'path');
    }
  }
}

function clearWalls() {
  if (isRunning) return;
  for (const row of grid) for (const n of row) {
    if (n.isWall) { n.isWall = false; cellEl(n).classList.remove('wall'); }
  }
  resetStats();
}

function resetBoard() {
  if (isRunning) return;
  buildGrid();
  resetStats();
}

function resetStats() {
  statVisited.textContent = '—';
  statPath.textContent = '—';
  statStatus.textContent = 'Ready';
  statStatus.className = '';
}

/* ---------------------------- visualization ------------------------------ */
const SPEEDS = { fast: 6, medium: 18, slow: 42 };

function sleep(ms) { return new Promise(res => setTimeout(res, ms)); }

async function visualize() {
  if (isRunning) return;
  softReset();
  isRunning = true;
  setControls(false);
  statStatus.textContent = 'Searching…';
  statStatus.className = '';

  const start = findNode('isStart');
  const finish = findNode('isFinish');
  const { visitedOrder, path } = ALGORITHMS[algoSel.value](grid, start, finish);

  const delay = SPEEDS[speedSel.value];
  // animate expansion
  for (const node of visitedOrder) {
    if (!node.isStart && !node.isFinish) cellEl(node).classList.add('visited');
    statVisited.textContent = visitedOrder.indexOf(node) + 1;
    if (delay) await sleep(delay);
  }
  statVisited.textContent = visitedOrder.length;

  // animate final path
  if (path.length && path[path.length - 1] === finish) {
    for (const node of path) {
      if (!node.isStart && !node.isFinish) cellEl(node).classList.add('path');
      await sleep(28);
    }
    statPath.textContent = path.length - 1; // edges
    statStatus.textContent = 'Path found';
    statStatus.className = 'ok';
  } else {
    statPath.textContent = '—';
    statStatus.textContent = 'No path (blocked)';
    statStatus.className = 'bad';
  }

  isRunning = false;
  setControls(true);
}

function setControls(enabled) {
  document.querySelectorAll('button, select').forEach(el => { el.disabled = !enabled; });
}

/* ------------------------------ presets ---------------------------------- */
// Scatter random walls (~25% density) so there's an obstacle field to route
// around. Clears any previous walls and stale visited/path styling first.
function randomWalls() {
  if (isRunning) return;
  clearWalls();
  softReset();
  for (const row of grid) for (const n of row) {
    if (!n.isStart && !n.isFinish && Math.random() < 0.20) {
      n.isWall = true;
      cellEl(n).classList.add('wall');
    }
  }
}

/* ------------------------------- wiring ---------------------------------- */
document.getElementById('run').addEventListener('click', visualize);
document.getElementById('clear-walls').addEventListener('click', clearWalls);
document.getElementById('reset').addEventListener('click', resetBoard);
document.getElementById('maze').addEventListener('click', randomWalls);

buildGrid();
resetStats();
