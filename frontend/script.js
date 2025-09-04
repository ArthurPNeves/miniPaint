const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");
const algoritmoSelect = document.getElementById("algoritmo");

let clicks = [];
let objetos = [];
let selecionados = [];

// === Funções utilitárias ===
function drawObjeto(obj) {
  ctx.fillStyle = obj.selecionado ? "red" : "blue";
  obj.pixels.forEach(p => {
    const x = p.x ?? p[0];
    const y = p.y ?? p[1];
    ctx.fillRect(x, y, 1, 1);
  });
}


function addObjeto(obj) {
  objetos.push(obj);
  drawObjeto(obj); // só desenha ele
}

function atualizarCanvas() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  objetos.forEach(drawObjeto);
}

function limparCanvas() {
  objetos = [];
  selecionados = [];
  ctx.clearRect(0, 0, canvas.width, canvas.height);
}

function getModo() {
  return document.querySelector('input[name="modo"]:checked').value;
}

// === Criação de objetos ===
canvas.addEventListener("click", async (e) => {
  if (getModo() !== "desenho") return;

  const rect = canvas.getBoundingClientRect();
  const x = Math.floor(e.clientX - rect.left);
  const y = Math.floor(e.clientY - rect.top);

  clicks.push({ x, y });
  ctx.fillStyle = "black";
  ctx.fillRect(x, y, 3, 3);

  const algoritmo = algoritmoSelect.value;

  if (algoritmo === "bresenham_circulo" && clicks.length === 2) {
    const dx = clicks[1].x - clicks[0].x;
    const dy = clicks[1].y - clicks[0].y;
    const r = Math.floor(Math.sqrt(dx * dx + dy * dy));
    await sendDraw({ algoritmo, xc: clicks[0].x, yc: clicks[0].y, r }, "circulo");
    clicks = [];
  } else if (clicks.length === 2) {
    await sendDraw(
      { algoritmo, x1: clicks[0].x, y1: clicks[0].y, x2: clicks[1].x, y2: clicks[1].y },
      "linha"
    );
    clicks = [];
  }
});

async function sendDraw(payload, tipo) {
  const res = await fetch("http://localhost:8080/draw", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });
  const data = await res.json();
  addObjeto({
    tipo: data.tipo,
    dados: data.dados,
    pixels: data.pixels,
    selecionado: false,
  });
}

// === Seleção de objetos ===
let selecting = false,
  selectStart = null,
  selectEnd = null;

canvas.addEventListener("mousedown", (e) => {
  if (getModo() !== "selecao") return;
  const rect = canvas.getBoundingClientRect();
  selectStart = { x: e.clientX - rect.left, y: e.clientY - rect.top };
  selecting = true;
});

canvas.addEventListener("mousemove", (e) => {
  if (getModo() !== "selecao" || !selecting) return;
  const rect = canvas.getBoundingClientRect();
  selectEnd = { x: e.clientX - rect.left, y: e.clientY - rect.top };
  atualizarCanvas();
  drawSelectionBox();
});

canvas.addEventListener("mouseup", () => {
  if (getModo() !== "selecao") return;
  if (selecting && selectEnd) selecionarObjetos();
  selecting = false;
  selectStart = null;
  selectEnd = null;
});

function drawSelectionBox() {
  if (!selectStart || !selectEnd) return;
  ctx.strokeStyle = "green";
  ctx.strokeRect(
    selectStart.x,
    selectStart.y,
    selectEnd.x - selectStart.x,
    selectEnd.y - selectStart.y
  );
}

function selecionarObjetos() {
  if (!selectStart || !selectEnd) return;
  const x1 = Math.min(selectStart.x, selectEnd.x);
  const y1 = Math.min(selectStart.y, selectEnd.y);
  const x2 = Math.max(selectStart.x, selectEnd.x);
  const y2 = Math.max(selectStart.y, selectEnd.y);

  selecionados = [];
  objetos.forEach((obj) => {
    const dentro = obj.pixels.some(p => {
      const px = p.x ?? p[0];
      const py = p.y ?? p[1];
      return px >= x1 && px <= x2 && py >= y1 && py <= y2;
    });
    obj.selecionado = dentro;
    if (dentro) selecionados.push(obj);
  });
  atualizarCanvas();
}


// === Transformações ===


function coletarParametrosTransformacao(transf) {
  let params = {};
  if (transf==="translacao"){
    params.dx=parseInt(document.getElementById("dx").value);
    params.dy=parseInt(document.getElementById("dy").value);
  }
  else if (transf==="escala"){
    params.sx=parseFloat(document.getElementById("sx").value);
    params.sy=parseFloat(document.getElementById("sy").value);
  }
  else if (transf==="rotacao"){
    params.angulo=parseFloat(document.getElementById("angulo").value);
  }
  else if (transf==="reflexao"){
    params.eixo=document.getElementById("eixo").value;
  }
  return params;
}

async function aplicarTransformacao(transf) {
  if (selecionados.length === 0) return alert("Selecione um objeto!");

  const params = coletarParametrosTransformacao(transf);

  for (let obj of selecionados) {
    const res = await fetch("http://localhost:8080/transform", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({tipo: obj.tipo, dados: obj.dados, transf, params})
    });
    const data = await res.json();
    obj.dados = data.dados;
    obj.pixels = data.pixels;
  }

  atualizarCanvas();
}

