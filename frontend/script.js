const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");
const algoritmoSelect = document.getElementById("algoritmo");

let clicks = [];
let objetos = [];
let selecionados = [];

// === Funções utilitárias ===
function drawObjeto(obj) {
  ctx.fillStyle = obj.selecionado ? "red" : "blue";

  // evita crash se pixels não existir
  if (!obj.pixels || !Array.isArray(obj.pixels)) {
    console.warn("drawObjeto: obj.pixels ausente ou inválido para", obj);
    return;
  }

  obj.pixels.forEach(p => {
    const x = p.x ?? p[0];
    const y = p.y ?? p[1];
    ctx.fillRect(x, y, 1, 1);
  });
}



function addObjeto(obj) {
  // garante que sempre exista um array para pixels (evita crash e facilita debug)
  if (!obj.pixels || !Array.isArray(obj.pixels)) {
    console.warn("addObjeto: pixels indefinido — inicializando array vazio", obj);
    obj.pixels = [];
  }
  objetos.push(obj);
  drawObjeto(obj); // só desenha ele
}


// novo atualizarCanvas com suporte à "janela de recorte"
function atualizarCanvas() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  // desenha todos os objetos normalmente (estado intacto)
  objetos.forEach(drawObjeto);

  // se há uma janela de recorte ativa, desenha overlay e redesenha a região interna
  if (window.cropRect) {
    const r = window.cropRect;
    // overlay escuro
    ctx.save();
    ctx.fillStyle = "rgba(0,0,0,0.45)";
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.restore();

    // redesenha apenas a região da janela por cima do overlay (clip)
    ctx.save();
    ctx.beginPath();
    ctx.rect(r.x, r.y, r.w, r.h);
    ctx.clip();
    objetos.forEach(drawObjeto); // só a parte dentro do clip será desenhada por cima do overlay
    ctx.restore();

    // borda da janela
    ctx.save();
    ctx.lineWidth = 2;
    ctx.strokeStyle = "#FFD700";
    ctx.strokeRect(r.x + 0.5, r.y + 0.5, r.w - 1, r.h - 1);
    ctx.restore();
  }
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
  try {
    const res = await fetch("http://localhost:8080/draw", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });

    if (!res.ok) {
      const text = await res.text().catch(()=>"");
      throw new Error(`Servidor retornou status ${res.status}. Body: ${text}`);
    }

    const data = await res.json();
    console.log("sendDraw: resposta /draw ->", data);

    // Se o servidor não retornou pixels, tenta obter rasterização a partir de 'dados' (fallback)
    let pixels = data.pixels;
    if (!pixels) {
      console.warn("sendDraw: data.pixels ausente, tentando rasterizar usando data.dados...");
      // chamamos /draw novamente enviando apenas os dados do objeto (backend deve rasterizar)
      const fallbackBody = data.dados && Object.keys(data.dados).length ? data.dados : payload;
      const res2 = await fetch("http://localhost:8080/draw", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(fallbackBody),
      });

      if (res2.ok) {
        const d2 = await res2.json();
        console.log("sendDraw: resposta fallback ->", d2);
        pixels = d2.pixels ?? [];
      } else {
        console.error("sendDraw: fallback /draw falhou com status", res2.status);
        pixels = [];
      }
    }

    addObjeto({
      tipo: data.tipo || tipo || "linha",
      dados: data.dados || payload,
      pixels: pixels,
      selecionado: false,
    });

  } catch (err) {
    console.error("sendDraw error:", err);
    alert("Erro ao comunicar com o servidor: " + err.message + ". Verifique console/server.");
  }
}


// === Seleção de objetos ===
let selecting = false,
  selectStart = null,
  selectEnd = null;

canvas.addEventListener("mousedown", (e) => {
  if (!["selecao", "recorte"].includes(getModo())) return;
  const rect = canvas.getBoundingClientRect();
  selectStart = { x: e.clientX - rect.left, y: e.clientY - rect.top };
  selecting = true;
});

canvas.addEventListener("mousemove", (e) => {
  if (!["selecao", "recorte"].includes(getModo()) || !selecting) return;
  const rect = canvas.getBoundingClientRect();
  selectEnd = { x: e.clientX - rect.left, y: e.clientY - rect.top };
  atualizarCanvas();
  drawSelectionBox();
});

canvas.addEventListener("mouseup", () => {
  if (!["selecao", "recorte"].includes(getModo())) return;
  selecting = false;
  if (getModo() === "selecao" && selectEnd) selecionarObjetos();
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

// novo aplicarRecorte (janela — NÃO altera objetos)
function aplicarRecorte() {
  if (!selectStart || !selectEnd) return alert("Selecione uma região!");

  const xmin = Math.min(selectStart.x, selectEnd.x);
  const ymin = Math.min(selectStart.y, selectEnd.y);
  const xmax = Math.max(selectStart.x, selectEnd.x);
  const ymax = Math.max(selectStart.y, selectEnd.y);

  const cropRect = { x: xmin, y: ymin, w: xmax - xmin, h: ymax - ymin };
  window.cropRect = cropRect;

  function objetoIntersectsRect(obj, rect) {
    if (obj.pixels && Array.isArray(obj.pixels) && obj.pixels.length) {
      let minx = Infinity, miny = Infinity, maxx = -Infinity, maxy = -Infinity;
      for (const p of obj.pixels) {
        const x = (p.x !== undefined) ? p.x : p[0];
        const y = (p.y !== undefined) ? p.y : p[1];
        if (x < minx) minx = x;
        if (y < miny) miny = y;
        if (x > maxx) maxx = x;
        if (y > maxy) maxy = y;
      }
      if (minx === Infinity) return false;
      return !(maxx < rect.x || minx > rect.x + rect.w || maxy < rect.y || miny > rect.y + rect.h);
    }
    if (obj.dados) {
      const d = obj.dados;
      if (obj.tipo === 'linha' && Array.isArray(d)) {
        const xs = d.map(p=>p.x!==undefined?p.x:p[0]);
        const ys = d.map(p=>p.y!==undefined?p.y:p[1]);
        const minx = Math.min(...xs), miny = Math.min(...ys), maxx = Math.max(...xs), maxy = Math.max(...ys);
        return !(maxx < rect.x || minx > rect.x + rect.w || maxy < rect.y || miny > rect.y + rect.h);
      }
      if (obj.tipo === 'circulo') {
        const cx = d.cx ?? d.x ?? (d[0]??0);
        const cy = d.cy ?? d.y ?? (d[1]??0);
        const r = d.r ?? d.radius ?? 0;
        const minx = cx - r, miny = cy - r, maxx = cx + r, maxy = cy + r;
        return !(maxx < rect.x || minx > rect.x + rect.w || maxy < rect.y || miny > rect.y + rect.h);
      }
    }
    return false;
  }

  for (let obj of objetos) {
    obj.selecionado = objetoIntersectsRect(obj, cropRect);
  }

  atualizarCanvas();
}

function resetRecorte() {
  // se temos snapshot original, restaura
  if (typeof originalObjetosSnapshot !== 'undefined' && originalObjetosSnapshot && originalObjetosSnapshot.length) {
    try {
      objetos = JSON.parse(JSON.stringify(originalObjetosSnapshot));
      console.log('resetRecorte: restaurado a partir do snapshot salvo.', objetos.length, 'objetos.');
    } catch (e) {
      objetos = (originalObjetosSnapshot || []).slice();
      console.warn('resetRecorte: deep copy falhou, usando cópia rasa', e);
    }
  } else {
    // fallback: remove quaisquer objetos gerados por recorte (isRecorte=true)
    objetos = (objetos || []).filter(o => !o.isRecorte);
    console.log('resetRecorte: removidos objetos de recorte (fallback).');
  }

  // limpa seleção e redesenha
  selecionados = [];
  window.cropRect = null;
  atualizarCanvas();

  // remover overlay de recorte se existir
  const overlay = document.getElementById('cropOverlay');
  if (overlay) overlay.remove();

  // resetar flags comuns usadas pelo recorte
  window.isCropping = false;
  window.cropRect = null;
  window.currentCrop = null;

  // voltar modo para desenho
  const modos = document.getElementsByName('modo');
  for (const m of modos) {
    if (m.value === 'desenho') { m.checked = true; break; }
  }

  // redesenhar shapes caso exista função
  if (typeof redrawShapes === 'function') redrawShapes();

  console.log('resetRecorte: estado restaurado.');
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

