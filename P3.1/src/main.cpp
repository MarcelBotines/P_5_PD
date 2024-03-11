#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "WifiProva";
const char* password = "123456789";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 File Manager</title>
  <style>
    body { font-family: Arial; }
    ul { list-style-type: none; }
    #fileInput { display: none; }
  </style>
</head>
<body>
  <h2>ESP32 File Manager</h2>
  <button onclick="document.getElementById('fileInput').click();">Subir Archivo</button>
  <input type="file" id="fileInput" onchange="uploadFile()" multiple>
 <div id="fileList"></div>


  <script>
  document.addEventListener('DOMContentLoaded', function (e) {
    fetch('/list').then(response => response.json())
      .then(data => {
        var fileList = document.getElementById('fileList');
        data.forEach(file => {
          var li = document.createElement('li');
          var a = document.createElement('a');
          a.href = `/download/${file}`;
          a.text = file;
          li.appendChild(a);
          fileList.appendChild(li);
        });
      });
  });

  function uploadFile() {
    var files = document.getElementById('fileInput').files;
    var formData = new FormData();
    for (var i = 0; i < files.length; i++) {
      formData.append('file', files[i]);
    }

    fetch('/upload', {method: 'POST', body: formData})
      .then(response => {
        if (response.ok) {
          console.log('Upload successful');
          location.reload();
        } else {
          console.error('Upload failed');
        }
      });
  }

 document.addEventListener('DOMContentLoaded', function () {
      fetch('/list')
        .then(response => response.json())
        .then(data => {
          var fileList = document.getElementById('fileList');
          data.forEach(file => {
            var a = document.createElement('a');
            a.href = `/download/${file}`;
            a.text = file;
            a.download = file;
            fileList.appendChild(a);
          });
        })
        .catch(error => {
          console.error('Error fetching file list:', error);
        });
    });
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "[";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file){
        if(json != "[") json += ',';
        json += "\"" + String(file.name()).substring(1) + "\"";
        file = root.openNextFile();
    }
    json += "]";
    request->send(200, "application/json", json);
    file.close();
  });

  // Upload a file to SPIFFS
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){},
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if(!index){
        Serial.printf("UploadStart: %s\n", filename.c_str());
        request->_tempFile = SPIFFS.open("/" + filename, "w");
      }
      if(len){
        request->_tempFile.write(data, len);
      }
      if(final){
        Serial.printf("UploadEnd: %s(%u)\n", filename.c_str(), index+len);
        request->_tempFile.close();
      }
  });

  // Download a file from SPIFFS
  server.on("^\\/download\\/(.*)$", HTTP_GET, [](AsyncWebServerRequest *request){
    String filename = request->pathArg(0);
    Serial.println("Download: " + filename);
    if(SPIFFS.exists("/" + filename)){
      request->send(SPIFFS, "/" + filename, String(), true);
    } else {
      request->send(404);
    }
  });
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP()); //Show ESP32 IP on serial
  server.begin();

}

void loop() {

}

