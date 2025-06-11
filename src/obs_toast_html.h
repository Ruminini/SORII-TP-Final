#ifndef OBS_TOAST_HTML_H
#define OBS_TOAST_HTML_H

#include <pgmspace.h>

const char HTML_TOAST_OBS[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OBS Credential Toast</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      overflow: hidden;
      background-color: rgba(0, 0, 0, 0);
      margin: 0;
      padding: 0;
    }

    .toast-container {
      position: fixed;
      top: 4vw;
      right: 4vw;
      width: 100vw;
      max-width: 90%;
      z-index: 1000;
      display: flex;
      flex-direction: column-reverse;
    }

    .toast {
      background-color: rgba(44, 44, 44, 0.9);
      color: white;
      padding: 3vw;
      border-radius: 2vw;
      box-shadow: 0 1vw 2vw rgba(0, 0, 0, 0.2);
      margin-bottom: 2vw;
      opacity: 0;
      transform: translateX(100%);
      animation: slideIn 0.5s forwards, fadeOut 0.5s forwards 12s;
      transition: opacity 0.5s, transform 0.5s;
      position: relative;
      word-wrap: break-word;
      border-left: 6vw solid #ff4d4d;
    }

    .toast-title {
      font-weight: bold;
      margin-bottom: 1vw;
      font-size: 2.4em;
    }

    .toast-content {
      font-size: 2em;
    }

    @keyframes slideIn {
      from {
        transform: translateX(100%);
        opacity: 0;
      }

      to {
        transform: translateX(0);
        opacity: 1;
      }
    }

    @keyframes fadeOut {
      from {
        opacity: 1;
        transform: translateX(0);
      }

      to {
        opacity: 0;
        transform: translateX(100%);
      }
    }
  </style>
</head>

<body>
  <div class="toast-container" id="toastContainer"></div>
  <script>
    const socket = new WebSocket('ws://192.168.4.1/ws');
    const toastContainer = document.getElementById('toastContainer');
    socket.onopen = function (event) { console.log('Conexión WebSocket abierta.'); };
    socket.onmessage = function (event) {
      try {
        const payload = JSON.parse(event.data);
        if (payload.type && payload.data) { createToast(payload.type, payload.data); }
      } catch (e) { console.error('Error al parsear JSON:', e); }
    };
    socket.onclose = function (event) {
      console.log('Conexión WebSocket cerrada:', event.code, event.reason);
    };
    socket.onerror = function (error) { console.error('Error WebSocket:', error); };

    function createToast(type, data) {
      const toast = document.createElement('div');
      toast.className = 'toast';

      let title = '';
      let borderColor = '#ccc';

      switch (type) {
        case 'connect':
          title = 'Nuevo dispositivo conectado';
          borderColor = '#4dff4d'; // verde
          break;
        case 'disconnect':
          title = 'Dispositivo desconectado';
          borderColor = '#ff4d4d'; // rojo
          break;
        case 'capture':
          title = '¡Credencial capturada!';
          borderColor = '#4d94ff'; // azul
          break;
        default:
          title = 'Evento';
          borderColor = '#ccc';
      }

      let content = '';
      for (const key in data) {
        if (data.hasOwnProperty(key)) {
          content += `<strong>${key}:</strong> ${data[key]}<br>`;
        }
      }

      toast.style.borderColor = borderColor;
      toast.innerHTML = `
      <div class="toast-title">${title}</div>
      <div class="toast-content">${content}</div>
    `;
      toastContainer.append(toast);
      toast.addEventListener('animationend', (e) => {
        if (e.animationName === 'fadeOut') { toast.remove(); }
      });
    }
  </script>
</body>

</html>
)rawliteral";

#endif