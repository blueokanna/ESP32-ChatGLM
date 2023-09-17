const char *html = R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <meta http-equiv="X-UA-Compatible" content="ie=edge" />
    <title>BlueVale's Chat AI Charactor Bot</title>
    <link
      rel="icon"
      type="image/x-icon"
      href="https://cdn-icons-png.flaticon.com/512/724/724715.png?ga=GA1.1.1207828738.1694336906"
      style="color: transparent"
    />

    <style>
      body {
        background-image: url("https://images.pexels.com/photos/5865634/pexels-photo-5865634.jpeg");
        background-size: cover;
        background-repeat: no-repeat;

        font-family: Arial, sans-serif;
        color: #333;
        margin: 0;
        padding: 0;
      }

      .dark-mode {
        border-radius: 25px;
        color: #e3e3e3;
      }

      .dark-mode .chat-container {
        width: 780px;
        height: auto;
        padding: 10px;
        border-radius: 20px;
        background-color: rgba(0, 0, 0, 0.5);
        box-shadow: 0 0 10px rgba(255, 255, 255, 0.1);
      }

      .dark-mode h1 {
        color: #6ec6ff;
      }

      p {
        color: #ffffff;
        margin-bottom: 20px;
      }

      h3 {
        display: block;
        font-size: 1.17em;
        margin-top: 1em;
        margin-bottom: 1em;
        margin-left: 0;
        margin-right: 0;
        font-weight: bold;
        color: skyblue;
      }

      .chat-popup {
        display: flex;
        justify-content: center;
        align-items: center;
        min-height: 100vh;
      }

      .form-container {
        width: auto;
        background-color: rgba(175, 236, 255, 0.281);
        padding: 10px;
        border-radius: 10px;
        box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
        align-items: center;
      }

      textarea::placeholder {
        color: white;
      }

      textarea {
        width: 100%;
        height: 100vh;
        padding: px;
        margin-bottom: 15px;
        border: 1px solid #ddd;
        border-radius: 10px;
        font-size: 16px;
        resize: vertical;
      }
      #input-box {
        height: auto;
        text-align: start;
        background-color: transparent;
        color: aliceblue;
        padding: 8px;
        margin: 0 10px 0 0;
        font-size: 16px;
        border: 2px solid #adadad;
        border-radius: 13px;
      }

      .chat-box {
        display: flex;
        flex-direction: unset;
        align-items: baseline;
        justify-content: space-between;
      }
      #user_message_box {
        max-height: 100px;
        text-align: right;
      }
      #receive-box {
        height: 250px;
        text-align: start, center;
      }

      .btn {
        background-color: #6ec6ff;
        color: #131313;
        font-size: 16px;
        padding: 10px 20px;
        border: none;
        border-radius: 8px;
        cursor: pointer;
        width: 100%;
        font-weight: bold;
      }

      #send-button {
        width: 30%;
        align-self: start;
      }
      .btn:hover {
        background-color: #0050a5ec;
      }

      #waiting-circle {
        height: auto;
        border: 6px solid #f3f3f3;
        border-top: 6px solid #3498db;
        border-radius: 50%;
        width: 50px;
        height: 50px;
        align-self: center;
        margin: 0 auto;
        animation: spin 8s linear infinite;
      }

      @keyframes spin {
        0% {
          transform: rotate(0deg);
        }
        100% {
          transform: rotate(360deg);
        }
      }
    </style>
  </head>

  <body class="dark-mode">
    <div class="chat-popup" id="myForm">
      <div class="chat-container">
        <h1>BlueVale's Q&A Artificial Intelligence Chat</h1>
        <p>Have a light platform conversation by ChatGLM AI</p>
        <div class="form-container">
          <h3>Chat with BlueVale's AI</h3>
          <div
            id="user_message_box"
            class="chat-box-container"
            style="overflow: auto"
          ></div>
          <br />
          <div id="waiting-circle"></div>
          <div
            id="receive-box"
            class="chat-box-container"
            style="overflow: auto"
          ></div>

          <br />
          <div class="chat-box">
            <textarea
              id="input-box"
              tabindex="0"
              data-id="root"
              required
              placeholder="Type message..."
            ></textarea>
            <button id="send-button" class="btn" data-id="root">Send</button>
          </div>
          <br />
        </div>
      </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/marked/marked.min.js"></script>
    <script>
      const inputBox = document.getElementById("input-box");
      const userMessageElement = document.getElementById("user_message_box");
      const receiveBox = document.getElementById("receive-box");
      const sendButton = document.getElementById("send-button");
      const waiting_circle = document.getElementById("waiting-circle");

      let message;
      let debounceTimer;
      let intervalId;
      waiting_circle.style.display = "none";

      sendButton.addEventListener("click", async () => {
        message = inputBox.value.trim();
        if (message !== "") {
          try {
            const response = await fetch(
              "/send?message=" + encodeURIComponent(message)
            );
            waiting_circle.style.display = "flex";
            if (!response.ok) {
              throw new Error(`HTTP error! Status: ${response.status}`);
            }

            inputBox.value = "";
            if (debounceTimer) {
              clearTimeout(debounceTimer);
            }
            debounceTimer = setTimeout(pollForResponse, 9000);
          } catch (error) {
            console.error("Error:", error);
          }
        }
      });

      function pollForResponse() {
        intervalId = setInterval(async () => {
          try {
            const response = await fetch("/receiveTextMessage");

            if (!response.ok) {
              throw new Error(`HTTP error! Status: ${response.status}`);
            }

            const data = await response.json();

            if (data.data.choices && data.data.choices.length > 0) {
              waiting_circle.style.display = "none";
              const aiResponse = data.data.choices[0].content
                .replace(/^\"\s([\s\S]+)\"$/, "$1")
                .replaceAll("\\n", "\n")
                .replaceAll('\\"', '"');
              displayMessages(message, aiResponse);
              clearInterval(intervalId);
            } else {
              waiting_circle.style.display = "flex";
              console.error("No choices found in the response.");
            }
          } catch (error) {
            console.error("Error:", error);
          }
        }, 3000);
      }

      function displayMessages(userMessage, aiResponse) {
        userMessageElement.innerHTML = marked.parse(userMessage);
        receiveBox.innerHTML = marked.parse(aiResponse);
      }
    </script>
  </body>
</html>
)";
