const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");
const algoritmoSelect = document.getElementById("algoritmo");
let objetosRecortados = null;           // camada de preview mostrada após aplicarRecorte()
let originalObjetosSnapshot = null;     // snapshot para undo (opcional)

// Inicialização da interface
document.addEventListener('DOMContentLoaded', function() {
  initializeInterface();
});

function initializeInterface() {
  setupThemeToggle();
  setupFullscreen();
  
  // Carrega tema salvo
  const savedTheme = localStorage.getItem('paint-theme') || 'light';
  if (savedTheme === 'dark') {
    document.documentElement.setAttribute('data-theme', 'dark');
    document.getElementById('themeToggle').querySelector('.theme-icon').textContent = '☀️';
  }
}

function setupThemeToggle() {
  const themeToggle = document.getElementById('themeToggle');
  const themeIcon = themeToggle.querySelector('.theme-icon');
  
  themeToggle.addEventListener('click', function() {
    const currentTheme = document.documentElement.getAttribute('data-theme');
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
    
    document.documentElement.setAttribute('data-theme', newTheme);
    themeIcon.textContent = newTheme === 'dark' ? '☀️' : '🌙';
    
    localStorage.setItem('paint-theme', newTheme);
    
    // Redesenha o canvas para ajustar as cores
    atualizarCanvas();
    
    // Animação de feedback
    this.style.transform = 'scale(0.95)';
    setTimeout(() => {
      this.style.transform = '';
    }, 150);
  });
}

function setupFullscreen() {
  const fullscreenBtn = document.getElementById('fullscreenBtn');
  const canvasFullscreenBtn = document.getElementById('canvasFullscreen');
  const fullscreenOverlay = document.getElementById('fullscreenOverlay');
  const fullscreenClose = document.getElementById('fullscreenClose');
  const fullscreenCanvas = document.getElementById('fullscreenCanvas');
  
  function openFullscreen() {
    updateStatus('Abrindo tela cheia...', 'warning');
    
    // Copia o conteúdo do canvas principal para o canvas fullscreen
    fullscreenCanvas.width = canvas.width;
    fullscreenCanvas.height = canvas.height;
    const fullscreenCtx = fullscreenCanvas.getContext('2d');
    fullscreenCtx.drawImage(canvas, 0, 0);
    
    fullscreenOverlay.classList.add('active');
    document.body.style.overflow = 'hidden';
    
    setTimeout(() => {
      updateStatus('Tela cheia ativada', 'success');
    }, 300);
  }
  
  function closeFullscreen() {
    fullscreenOverlay.classList.remove('active');
    document.body.style.overflow = 'auto';
    
    // Copia de volta as alterações do canvas fullscreen para o principal
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.drawImage(fullscreenCanvas, 0, 0);
    
    updateStatus('Tela cheia fechada', 'info');
  }
  
  fullscreenBtn.addEventListener('click', openFullscreen);
  canvasFullscreenBtn.addEventListener('click', openFullscreen);
  fullscreenClose.addEventListener('click', closeFullscreen);
  
  // Fecha com ESC
  document.addEventListener('keydown', function(e) {
    if (e.key === 'Escape' && fullscreenOverlay.classList.contains('active')) {
      closeFullscreen();
    }
  });
  
  // Fecha clicando fora do canvas
  fullscreenOverlay.addEventListener('click', function(e) {
    if (e.target === fullscreenOverlay) {
      closeFullscreen();
    }
  });
}

function updateStatus(message, type = 'info') {
  const statusIndicator = document.getElementById('statusIndicator');
  const statusText = document.getElementById('statusText');
  
  statusText.textContent = message;
  
  // Remove classes anteriores
  statusIndicator.className = 'status-indicator';
  
  // Adiciona classe baseada no tipo
  switch(type) {
    case 'success':
      statusIndicator.style.background = 'var(--success-color)';
      statusIndicator.style.boxShadow = '0 0 8px var(--success-color)';
      break;
    case 'warning':
      statusIndicator.style.background = 'var(--warning-color)';
      statusIndicator.style.boxShadow = '0 0 8px var(--warning-color)';
      break;
    case 'error':
      statusIndicator.style.background = 'var(--danger-color)';
      statusIndicator.style.boxShadow = '0 0 8px var(--danger-color)';
      break;
    default:
      statusIndicator.style.background = 'var(--accent-color)';
      statusIndicator.style.boxShadow = '0 0 8px var(--accent-color)';
  }
  
  // Auto-hide after 3 seconds
  setTimeout(() => {
    statusText.textContent = 'Pronto';
    statusIndicator.style.background = 'var(--success-color)';
    statusIndicator.style.boxShadow = '0 0 8px var(--success-color)';
  }, 3000);
}

// Sistema de coordenadas cartesiano (4 quadrantes)
const CANVAS_CENTER_X = canvas.width / 2;
const CANVAS_CENTER_Y = canvas.height / 2;

// Conversão entre sistema cartesiano e canvas
function cartesianToCanvas(x, y) {
  return {
    x: CANVAS_CENTER_X + x,
    y: CANVAS_CENTER_Y - y  // inverte Y pois canvas tem origem no topo
  };
}

function canvasToCartesian(canvasX, canvasY) {
  return {
    x: canvasX - CANVAS_CENTER_X,
    y: CANVAS_CENTER_Y - canvasY  // inverte Y
  };
}

let clicks = [];
let objetos = [];
let selecionados = [];

// === Desenho ===

function drawObjeto(obj) {
  // Prioridade: se houver pixels (preview ou raster), desenhe-os
  if (obj.pixels && Array.isArray(obj.pixels) && obj.pixels.length > 0) {
    const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
    ctx.fillStyle = obj.selecionado ? 'red' : (isDark ? '#f1f5f9' : 'black');
    for (const p of obj.pixels) {
      // Converte coordenadas cartesianas para canvas
      const canvasPos = cartesianToCanvas(p.x, p.y);
      ctx.fillRect(Math.round(canvasPos.x), Math.round(canvasPos.y), 1, 1);
    }
    return;
  }
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

function atualizarCanvas() {
  // limpa canvas
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  // Desenha os eixos cartesianos
  drawCartesianAxes();

  // SEMPRE desenha todos os objetos originais primeiro
  for (const obj of objetos) {
    drawObjeto(obj);
  }

  // Se houver preview de recorte, desenha por cima com cor diferente
  if (objetosRecortados && objetosRecortados.length > 0) {
    const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
    ctx.fillStyle = isDark ? '#60a5fa' : 'blue'; // cor diferente para preview
    for (const obj of objetosRecortados) {
      if (obj.pixels && Array.isArray(obj.pixels)) {
        for (const p of obj.pixels) {
          const canvasPos = cartesianToCanvas(p.x, p.y);
          ctx.fillRect(Math.round(canvasPos.x), Math.round(canvasPos.y), 2, 2); // um pouco maior para destacar
        }
      }
    }
  }
}

function drawCartesianAxes() {
  const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
  
  ctx.strokeStyle = isDark ? '#475569' : '#ddd';
  ctx.lineWidth = 1;
  
  // Eixo X (horizontal)
  ctx.beginPath();
  ctx.moveTo(0, CANVAS_CENTER_Y);
  ctx.lineTo(canvas.width, CANVAS_CENTER_Y);
  ctx.stroke();
  
  // Eixo Y (vertical)
  ctx.beginPath();
  ctx.moveTo(CANVAS_CENTER_X, 0);
  ctx.lineTo(CANVAS_CENTER_X, canvas.height);
  ctx.stroke();
  
  // Marca a origem
  ctx.fillStyle = isDark ? '#ef4444' : 'red';
  ctx.fillRect(CANVAS_CENTER_X - 2, CANVAS_CENTER_Y - 2, 4, 4);
  
  // Adiciona algumas marcas nos eixos
  ctx.fillStyle = isDark ? '#cbd5e1' : '#666';
  ctx.font = '12px Arial';
  ctx.textAlign = 'center';
  
  // Marcas no eixo X
  for (let i = -400; i <= 400; i += 100) {
    if (i !== 0) {
      const canvasPos = cartesianToCanvas(i, 0);
      ctx.fillText(i.toString(), canvasPos.x, canvasPos.y + 15);
    }
  }
  
  // Marcas no eixo Y
  ctx.textAlign = 'right';
  for (let i = -250; i <= 250; i += 50) {
    if (i !== 0) {
      const canvasPos = cartesianToCanvas(0, i);
      ctx.fillText(i.toString(), canvasPos.x - 10, canvasPos.y + 4);
    }
  }
  
  // Label da origem
  ctx.textAlign = 'left';
  ctx.fillText('(0,0)', CANVAS_CENTER_X + 5, CANVAS_CENTER_Y - 5);
}

// Desenha os eixos inicialmente
drawCartesianAxes();

function limparCanvas() {
  objetos = [];
  selecionados = [];
  objetosRecortados = null;
  originalObjetosSnapshot = null;
  tempRect = null;
  selectStart = null;
  selectEnd = null;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  drawCartesianAxes();
}

function getModo() {
  return document.querySelector('input[name="modo"]:checked').value;
}

// === Criação de objetos ===
canvas.addEventListener("click", async (e) => {
  if (getModo() !== "desenho") return;

  const rect = canvas.getBoundingClientRect();
  const canvasX = Math.floor(e.clientX - rect.left);
  const canvasY = Math.floor(e.clientY - rect.top);
  
  // Converte para coordenadas cartesianas
  const cartesian = canvasToCartesian(canvasX, canvasY);
  const x = cartesian.x;
  const y = cartesian.y;

  clicks.push({ x, y });
  
  // Desenha o ponto de clique no canvas (convertendo de volta)
  const canvasPos = cartesianToCanvas(x, y);
  const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
  ctx.fillStyle = isDark ? "#f1f5f9" : "black";
  ctx.fillRect(canvasPos.x, canvasPos.y, 3, 3);

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
    updateStatus('Desenhando...', 'warning');
    
    const res = await fetch("http://localhost:8080/draw", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });

    if (!res.ok) {
      const text = await res.text().catch(() => "");
      throw new Error(`Servidor retornou status ${res.status}. Body: ${text}`);
    }

    const data = await res.json();
    console.log("sendDraw response:", data);

    addObjeto({
      tipo,
      dados: payload,
      pixels: data.pixels || [],
      selecionado: false,
    });
    
    updateStatus(`${tipo} desenhado com sucesso`, 'success');

  } catch (err) {
    console.error("sendDraw error:", err);
    updateStatus('Erro ao desenhar', 'error');
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
  const canvasX = e.clientX - rect.left;
  const canvasY = e.clientY - rect.top;
  
  // Converte para coordenadas cartesianas
  const cartesian = canvasToCartesian(canvasX, canvasY);
  selectStart = { x: cartesian.x, y: cartesian.y };
  selecting = true;
});

canvas.addEventListener("mousemove", (e) => {
  if (!["selecao", "recorte"].includes(getModo()) || !selecting) return;
  const rect = canvas.getBoundingClientRect();
  const canvasX = e.clientX - rect.left;
  const canvasY = e.clientY - rect.top;
  
  // Converte para coordenadas cartesianas
  const cartesian = canvasToCartesian(canvasX, canvasY);
  selectEnd = { x: cartesian.x, y: cartesian.y };
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
  
  // Converte coordenadas cartesianas para canvas para desenhar
  const startCanvas = cartesianToCanvas(selectStart.x, selectStart.y);
  const endCanvas = cartesianToCanvas(selectEnd.x, selectEnd.y);
  
  const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
  ctx.strokeStyle = isDark ? "#10b981" : "green";
  ctx.strokeRect(
    Math.min(startCanvas.x, endCanvas.x),
    Math.min(startCanvas.y, endCanvas.y),
    Math.abs(endCanvas.x - startCanvas.x),
    Math.abs(endCanvas.y - startCanvas.y)
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

// === Recorte ===
async function aplicarRecorte() {
  // pega as coordenadas do retângulo de seleção
  if (!selectStart || !selectEnd) return alert("Selecione uma área para recortar.");
  const xmin = Math.min(selectStart.x, selectEnd.x);
  const ymin = Math.min(selectStart.y, selectEnd.y);
  const xmax = Math.max(selectStart.x, selectEnd.x);
  const ymax = Math.max(selectStart.y, selectEnd.y);
  const w = xmax - xmin;
  const h = ymax - ymin;
  if (w === 0 || h === 0) return alert("Área de recorte inválida.");

  // pega o algoritmo selecionado
  const algoritmoRecorte = document.getElementById("recorteAlgoritimo").value;

  // pega os objetos que têm pixels dentro do retângulo
  const recortados = objetos.filter(obj =>
    obj.pixels && obj.pixels.some(p => {
      const px = p.x ?? p[0];
      const py = p.y ?? p[1];
      return px >= xmin && px <= xmax && py >= ymin && py <= ymax;
    })
  );

  // se nenhum objeto for recortado, avisa e sai
  if (recortados.length === 0) {
    return alert("Nenhum objeto encontrado na área selecionada.");
  }

  // se for encontrado circulo, avisa que não é suportado
  if (recortados.some(o => o.tipo === 'circulo')) {
    return alert("Recorte de círculos não é suportado.");
  }

  // faz snapshot para possível undo (opcional)
  originalObjetosSnapshot = JSON.parse(JSON.stringify(objetos));

  // envia requisições individuais para o backend, conforme esperado pelo backend C++
  try {
    const resultados = [];
    for (const obj of recortados) {
      const reqBody = {
        tipo: obj.tipo,
        dados: obj.dados,
        algoritmo: algoritmoRecorte,
        xmin,
        ymin,
        xmax,
        ymax
      };
      const res = await fetch("http://localhost:8080/clip", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(reqBody)
      });
      if (!res.ok) {
        const text = await res.text().catch(() => "");
        throw new Error(`Servidor retornou status ${res.status}. Body: ${text}`);
      }
      const data = await res.json();
      if (data.aceita && data.pixels && data.pixels.length > 0) {
        resultados.push({
          tipo: obj.tipo,
          dados: obj.dados,
          pixels: data.pixels,
          selecionado: false
        });
      }
    }

    if (resultados.length === 0) {
      alert("Nenhum objeto foi recortado (nenhum pixel dentro da área).");
      return;
    }

    // cria camada de preview com os objetos recortados
    objetosRecortados = resultados;

    // atualiza canvas para mostrar a camada de preview
    atualizarCanvas();

    // limpa seleção e variáveis temporárias
    selectStart = null;
    selectEnd = null;
    tempRect = null;

  } catch (err) {
    console.error("aplicarRecorte error:", err);
    alert("Erro ao comunicar com o servidor: " + err.message + ". Verifique console/server.");
  }
}

async function resetRecorte() {
  if (!objetosRecortados) return alert("Nenhum recorte ativo.");
  objetosRecortados = null;
  originalObjetosSnapshot = null;
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
  if (selecionados.length === 0) {
    updateStatus('Nenhum objeto selecionado', 'warning');
    return alert("Selecione um objeto!");
  }

  updateStatus(`Aplicando ${transf}...`, 'warning');
  
  const params = coletarParametrosTransformacao(transf);

  try {
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

    updateStatus(`${transf} aplicada com sucesso`, 'success');
    atualizarCanvas();
  } catch (err) {
    console.error("Transform error:", err);
    updateStatus('Erro na transformação', 'error');
    alert("Erro ao aplicar transformação: " + err.message);
  }
}

// Função para feedback visual dos botões
function addButtonFeedback(button) {
  button.style.transform = 'scale(0.95)';
  setTimeout(() => {
    button.style.transform = '';
  }, 150);
}

// Adicionar feedback aos botões
document.addEventListener('DOMContentLoaded', function() {
  // Feedback para botões de transformação
  document.querySelectorAll('.paint-btn-small').forEach(btn => {
    btn.addEventListener('click', function() {
      addButtonFeedback(this);
    });
  });
  
  // Feedback para botões principais
  document.querySelectorAll('.paint-btn').forEach(btn => {
    btn.addEventListener('click', function() {
      addButtonFeedback(this);
    });
  });
  
  // Feedback para controles do menu
  document.querySelectorAll('.control-btn').forEach(btn => {
    btn.addEventListener('click', function() {
      addButtonFeedback(this);
    });
  });
});
