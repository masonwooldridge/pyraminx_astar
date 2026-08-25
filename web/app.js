const controls = document.querySelector('#controls');
const scrambleInput = document.querySelector('#scramble');
const status = document.querySelector('#status');
const solveButton = document.querySelector('#solve');
let engine;

const moves = ['U1','L1','R1','B1','U2','L2','R2','B2','U3','L3','R3','B3','U4','L4','R4','B4'];

function createPuzzle() {
  const puzzle = document.querySelector('#puzzle');
  for (let face = 0; face < 4; face += 1) {
    const element = document.createElement('div');
    element.className = 'face';
    element.setAttribute('aria-label', `Face ${face + 1}`);
    let index = 0;
    for (let row = 0; row < 4; row += 1) {
      for (let column = 0; column < row * 2 + 1; column += 1) {
        const sticker = document.createElement('i');
        sticker.className = `sticker ${'RGYB'[face]} ${column % 2 ? 'down' : ''}`;
        sticker.style.gridRow = row + 1;
        sticker.style.gridColumn = 4 - row + column;
        sticker.dataset.index = face * 16 + index++;
        element.append(sticker);
      }
    }
    puzzle.append(element);
  }
}

function call(name, args) {
  return engine.ccall(name, 'string', args.map(() => 'string'), args);
}

function draw(notation) {
  const colors = call('state_faces', [notation]);
  if (colors.startsWith('error:')) throw new Error(colors.slice(6));
  document.querySelectorAll('.sticker').forEach((sticker) => {
    sticker.classList.remove('R', 'G', 'Y', 'B');
    sticker.classList.add(colors[Number(sticker.dataset.index)]);
  });
}

function metric(id, value) { document.querySelector(`#${id}`).textContent = value; }
const wait = (milliseconds) => new Promise((resolve) => setTimeout(resolve, milliseconds));

async function animate(scramble, solution) {
  const steps = solution ? solution.split(' ') : [];
  let notation = scramble.trim();
  draw(notation);
  for (const step of steps) {
    await wait(260);
    notation = `${notation} ${step}`.trim();
    draw(notation);
  }
}

controls.addEventListener('submit', async (event) => {
  event.preventDefault();
  if (!engine) return;
  solveButton.disabled = true;
  status.textContent = 'Searching in WebAssembly…';
  await wait(20);
  const scramble = scrambleInput.value.trim();
  const result = JSON.parse(call('solve_scramble', [scramble,
    document.querySelector('#algorithm').value, document.querySelector('#heuristic').value]));
  if (!result.solved) {
    status.textContent = result.error || 'No solution within the resource limit.';
  } else {
    metric('solution', result.solution || 'Already solved');
    metric('length', result.length);
    metric('expanded', result.expanded.toLocaleString());
    metric('runtime', `${result.milliseconds.toFixed(2)} ms`);
    metric('frontier', result.peakFrontier.toLocaleString());
    metric('memory', `${(result.memoryBytes / 1024).toFixed(1)} KiB`);
    status.textContent = 'Optimal solution found and verified. Animating…';
    await animate(scramble, result.solution);
    status.textContent = 'Solved.';
  }
  solveButton.disabled = false;
});

document.querySelector('#randomize').addEventListener('click', () => {
  const chosen = [];
  while (chosen.length < 5) {
    let move = moves[Math.floor(Math.random() * moves.length)];
    if (Math.random() > .5) move += "'";
    const layer = (value) => value.replace("'", '');
    if (!chosen.length || layer(chosen.at(-1)) !== layer(move)) chosen.push(move);
  }
  scrambleInput.value = chosen.join(' ');
  if (engine) draw(scrambleInput.value);
});

createPuzzle();
if (typeof createPyraminxModule === 'function') {
  createPyraminxModule().then((module) => {
    engine = module;
    status.textContent = 'WebAssembly engine ready.';
    draw(scrambleInput.value);
  }).catch(() => { status.textContent = 'Build the WebAssembly bundle to enable solving (see README).'; });
} else {
  status.textContent = 'Build the WebAssembly bundle to enable solving (see README).';
}
