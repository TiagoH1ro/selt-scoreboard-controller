#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>

if(!SPIFFS.begin(true)) {
  Serial.println("Erro ao montar o sistema de arquivos SPIFFS");
  return;
}

// Configurações da rede Wi-Fi
const char* ssid = "Controlador Placar - SELT";
const char* password = "basquete";

// Inicializa o servidor web
WebServer server(80);

int teamAScore = 0;
int teamBScore = 0;

int teamAFaults = 0;
int teamBFaults = 0;

int currentPeriod = 0;

#define Aplus 21
#define Bplus 19

#define Afault 18
#define Bfault 5

#define timer 17
#define period 16

void setup() {
  Serial.begin(115200);

  pinSetup();

  WiFi.softAP(ssid, password);
  Serial.println("Access Point configurado");

  // Define o IP fixo
  IPAddress local_IP(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  MDNS.begin("placar");

  WiFi.softAPConfig(local_IP, gateway, subnet);

  // Define as rotas para os botões
  server.on("/", handleRoot);
  server.on("/add1A", []() {
    teamAScore += 1;
    pinPulse(1, Aplus);
    server.send(200, "text/html", handleVariables());
  });
  server.on("/add2A", []() {
    teamAScore += 2;
    pinPulse(2, Aplus);
    server.send(200, "text/html", handleVariables());
  });
  server.on("/add3A", []() {
    teamAScore += 3;
    pinPulse(3, Aplus);
    server.send(200, "text/html", handleVariables());
  });
  server.on("/add1B", []() {
    teamBScore += 1;
    pinPulse(1, Bplus);
    server.send(200, "text/html", handleVariables());
  });
  server.on("/add2B", []() {
    teamBScore += 2;
    pinPulse(2, Bplus);
    server.send(200, "text/html", handleVariables());
  });
  server.on("/add3B", []() {
    teamBScore += 3;
    pinPulse(3, Bplus);
    server.send(200, "text/html", handleVariables());
  });

  server.on("/timer", []() {
    pinPulse(1, timer);
    server.send(200, "text/html", handleVariables());
  });

  server.on("/resetPts", []() {
    resetPts();
    teamAScore = 0;
    teamBScore = 0;
    server.send(200, "text/html", handleVariables());
  });

  server.on("/period", []() {
    currentPeriod++;
    pinPulse(1, period);
    server.send(200, "text/html", handleVariables());
  });

  server.on("/faultA", []() {
    teamAFaults++;
    pinPulse(1, Afault);
    server.send(200, "text/html", handleVariables());
  });

  server.on("/faultB", []() {
    teamBFaults++;
    pinPulse(1, Bfault);
    server.send(200, "text/html", handleVariables());
  });

  server.begin();
  Serial.println("Servidor Iniciado");
}

void loop() {
  server.handleClient();
}

void pinSetup() {
  pinMode(Aplus, OUTPUT);
  pinMode(Bplus, OUTPUT);
  pinMode(Afault, OUTPUT);
  pinMode(Bfault, OUTPUT);
  pinMode(timer, OUTPUT);
  pinMode(period, OUTPUT);

  digitalWrite(Aplus, LOW);
  digitalWrite(Bplus, LOW);
  digitalWrite(Afault, LOW);
  digitalWrite(Bfault, LOW);
  digitalWrite(timer, LOW);
  digitalWrite(period, LOW);
}

void pinPulse(int numOfPulses, int PIN) {
  for (int i = 0; i < numOfPulses; i++) {
    digitalWrite(PIN, HIGH);
    delay(200);
    digitalWrite(PIN, LOW);
    delay(200);
  }
}

void resetPts() {
  digitalWrite(Aplus, HIGH);
  digitalWrite(Bplus, HIGH);
  delay(3000);
  digitalWrite(Aplus, LOW);
  digitalWrite(Bplus, LOW);
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="pt-br">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Controlador Placar - SELT</title>
      <style>
        @font-face {
            font-family: 'Seven Segment';
            src: url('fonts/sevenSegment.ttf') format('truetype');
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            background-color: #cccccc;
            font-family: Arial, sans-serif;
            height: 100vh;
        }

        /* Container principal */
        .scoreboard-container {
            width: 96vw;
            height: 42vh;
            background-color: #4c4c4c;
            border-radius: 5vw;
            margin: 1vh auto;
            position: relative;
            display: flex;
            justify-content: space-around;
            align-items: center;
            flex-wrap: wrap;
            padding: 4vw;

            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.5);
        }

        .team-a-container {
            width: 96vw;
            height: 20vh;
            background-color: #0487D9;
            border-radius: 5vw;
            margin: 1vh auto;
            position: relative;
            display: flex;
            flex-direction: column;
            align-items: center;
            flex-wrap: wrap;
            padding: 1vw;

            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.5);
        }

        .team-b-container {
            width: 96vw;
            height: 20vh;
            background-color: #2E4959;
            border-radius: 5vw;
            margin: 1vh auto;
            position: relative;
            display: flex;
            flex-direction: column;
            align-items: center;
            flex-wrap: wrap;
            padding-top: 7vw;

            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.5);
        }

        .btn-row {
            display: flex;
            gap: 10px;
            margin-top: 1vh;
        }

        .btn {
            background-color: red;
            color: white;
            margin: 1vh auto;
            position: relative;
            display: block;
            padding: 4vw 4vw;
            font-size: 4vw;
            font-weight: bold;
            border: none;
            border-radius: 2vw;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            transition: background-color 0.3s ease;
        }

        .btns-team-a {
            background-color: #005488;
            color: white;
            margin: 1vh auto;
            position: relative;
            display: block;
            padding: 4vw 4vw;
            font-size: 4vw;
            font-weight: bold;
            border: none;
            border-radius: 2vw;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            transition: background-color 0.3s ease;
        }

        .btns-team-b {
            background-color: #192830;
            color: white;
            margin: 1vh auto;
            position: relative;
            display: block;
            padding: 4vw 4vw;
            font-size: 4vw;
            font-weight: bold;
            border: none;
            border-radius: 2vw;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            transition: background-color 0.3s ease;
        }

        .reset-btn-1 {
            background-color: #D26262;
            color: white;
            margin: 1vh auto;
            padding: 4vw 4vw;
            font-size: 4vw;
            font-weight: bold;
            border: none;
            border-radius: 2vw;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            transition: background-color 0.3s ease;
            position: absolute;
            top: 60vh;
            right: 50%;
            transform: translateX(10%);
        }

        .reset-btn-2 {
            background-color: #D26262;
            color: white;
            margin: 1vh auto;
            padding: 4vw 4vw;
            font-size: 4vw;
            font-weight: bold;
            border: none;
            border-radius: 2vw;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            transition: background-color 0.3s ease;
            position: absolute;
            top: 60vh;
            left: 50%;
            transform: translateX(50%);
        }

        .score-block {
            position: absolute;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .score-box {
            background-color: #414141;
            border: 1.5vw solid white;
            border-radius: 2vw;
            display: flex;
            justify-content: center;
            align-items: center;
            width: 20vw;
            height: 18vw;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
        }

        .score-box-2 {
            background-color: #414141;
            border: 1.5vw solid white;
            border-radius: 2vw;
            display: flex;
            justify-content: center;
            align-items: center;
            width: 32vw;
            height: 18vw;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
        }

        .score-text {
            font-family: 'Seven Segment', sans-serif;
            font-size: 16vw;
            font-weight: bold;
            color: red;
        }

        .score-label {
            color: white;
            font-family: system-ui, 'Segoe UI', 'Open Sans', 'Helvetica Neue', sans-serif;
            font-size: 3.5vw;
            font-weight: bolder;
            margin-top: 1vw;
            margin-bottom: 1vw;
            text-align: center;
        }

        .team-label {
            color: white;
            font-family: system-ui, 'Segoe UI', 'Open Sans', 'Helvetica Neue', sans-serif;
            font-size: 6vw;
            font-weight: bolder;
            margin-top: 1vw;
            margin-bottom: 1vw;
            text-align: center;
        }

        .period-box {
            left: 50%;
            background-color: #373737;
            border: 1.5vw solid white;
            border-radius: 3vw;
            width: 15vw;
            height: 15vw;
            display: flex;
            justify-content: center;
            align-items: center;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
        }

        .period-text {
            font-family: 'Seven Segment', sans-serif;
            font-size: 10vw;
            font-weight: bold;
            color: red;
        }

        .timer-box {
            left: 50%;
            background-color: #373737;
            border: 1.5vw solid white;
            border-radius: 3vw;
            width: 30vw;
            height: 15vw;
            display: flex;
            justify-content: center;
            align-items: center;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
        }

        .timer-text {
            font-family: 'Seven Segment', sans-serif;
            font-size: 10vw;
            font-weight: bolder;
            color: red;
            animation: fadeTimer 5s infinite ease-in-out;
            transition: opacity 5s ease-in-out;
        }

        /* Animção Fade */
        @keyframes fadeTimer {
            0%,
            100% {
                opacity: 1;
            }

            50% {
                opacity: 0;
            }
        }

        #faults-position-a {
            top: 5%;
            left: 5%;
        }

        #faults-position-b {
            top: 5%;
            right: 5%;
        }

        #period-position {
            top: 5%;
        }

        #timer-position {
            top: 35%;
        }

        #score-position-a {
            bottom: 2%;
            left: 5%;
        }

        #score-position-b {
            bottom: 2%;
            right: 5%;
        }

        p {
            text-align: center;
        }

        a {
            color: inherit;       
            display: inline-flex;    
            justify-content: center;
            align-items: center; 
        }

        .github-icon {
            width: 20px;
            height: 20px;
            margin-right: 8px;
        }
      </style>
    </head>
    <body>
        <div class="scoreboard-container">
            <!-- FALTAS/SET A -->
            <div class="score-block" id="faults-position-a">
                <div class="score-box">
                    <span class="score-text">)rawliteral"
                    + String(teamAFaults) + R"rawliteral(</span>
                </div>
                <p class="score-label">FALTAS/SET A</p>
            </div>

            <!-- PERÍODO -->
            <div class="score-block" id="period-position">
                <span class="score-label">PERÍODO</span>
                <div class="period-box" >
                    <span class="period-text">)rawliteral"
                    + String(currentPeriod) + R"rawliteral(</span>
                </div>
            </div>

            <!-- FALTAS/SET B -->
            <div class="score-block" id="faults-position-b">
                <div class="score-box">
                    <span class="score-text">)rawliteral"
                    + String(teamBFaults) + R"rawliteral(</span>
                </div>
                <p class="score-label">FALTAS/SET B</p>
            </div>

            <!-- CRONÔMETRO -->
            <div class="score-block" id="timer-position">
                <p class="score-label">CRONÔMETRO</p>
                <div class="timer-box">
                    <span class="timer-text" id="timerDisplay">-- --</span>
                </div>
            </div>
            <!-- PONTOS TIME A -->
            <div class="score-block" id="score-position-a">
                <div class="score-box-2">
                    <span class="score-text">)rawliteral"
                    + String(teamAScore) + R"rawliteral(</span>
                </div>
                <p class="score-label">TIME A</p>
            </div>
            <!-- PONTOS TIME B -->
            <div class="score-block" id="score-position-b">
                <div class="score-box-2">
                    <span class="score-text">)rawliteral"
                    + String(teamBScore) + R"rawliteral(</span>
                </div>
                <p class="score-label">TIME B</p>
            </div>
        </div>

        <!-- Controles Time A -->
        <div class="team-a-container">
            <p class="team-label">TIME A</p>
            <div class="btn-row">
                <button class="btns-team-a" onclick="sendRequest('/add1A')">+ 1</button>
                <button class="btns-team-a" onclick="sendRequest('/add2A')">+ 2</button>
                <button class="btns-team-a" onclick="sendRequest('/add3A')">+ 3</button>
                <button class="btns-team-a" onclick="sendRequest('/faultA')">FALTA</button>
            </div>
        </div>

        <!-- Controles Time B -->
        <div class="team-b-container">
            <p class="team-label">TIME B</p>
            <div class="btn-row">
                <button class="btns-team-b" onclick="sendRequest('/add1B')">+ 1</button>
                <button class="btns-team-b" onclick="sendRequest('/add2B')">+ 2</button>
                <button class="btns-team-b" onclick="sendRequest('/add3B')">+ 3</button>
                <button class="btns-team-b" onclick="sendRequest('/faultB')">FALTA</button>
            </div>
        </div>

        <!-- Botão de resetar pontos -->
        <button class="reset-btn-1" onclick="sendRequest('/resetPts')">RESETAR PONTOS</button>
        <!-- Botão Período -->
        <button class="reset-btn-2" onclick="sendRequest('/period')">PERÍODO</button>
        
        <!-- Botão Cronômetro -->
        <button class="btn" onclick="sendRequest('/timer')">
            CRONÔMETRO
        </button>

        <!-- Rodapé -->
        <p>Feito por Tiago Oliveira. 
            <a href="https://github.com/TiagoH1ro">
                Github 
                <img src="/github.svg" class="github-icon">
            </a>
        </p>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

//TODO: implementar a única parte que mudará na página (AJAX?)
void handleVariables() {

}