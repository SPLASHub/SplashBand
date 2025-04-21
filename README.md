# Descrição
Protótipo de pulseira de monitoramento.

Utiliza um microcontrolador ESP32-S3-WROOM-1 N16R8 para coletar dados de um sensor GPS NEO-6M e enviar esses dados via BLE.
# Requisitos
### Hardware
- Microcontrolador: ESP32-S3-WROOM-1 N16R8
- Sensor GPS: NEO-6M 
### Software
- [PlatformIO](https://platformio.org/)
- Python (instalado com o PlatformIO)
- [ESP-IDF (Espressif IoT Development Framework)](https://docs.espressif.com/projects/esp-idf/en/v5.2.5/esp32s3/api-reference/index.html)
- Driver para o conversor USB-Serial do ESP32-S3-WROOM-1 N16R8 (CP210x , CH340, PL2303, etc)

# Submodulo PulseiraSplash de SplashBeachgoer
Este projeto é um submodulo do projeto [SplashBeachgoer](https://github.com/JoaoPNVieira/SplashBeachgoer.git). Para clonar o projeto SplashBeachgoer com os submodulos:

- clonar o repo SplashBeachgoer com os submodulos:
```bash
git clone --recurse-submodules https://github.com/JoaoPNVieira/SplashBeachgoer.git
# ou 
gh repo clone JoaoPNVieira/SplashBeachgoer -- --recurse-submodules
```
- adicionar os submodulos ao repo SplashBeachgoer local:
```bash
cd path/to/SplashBeachgoer
git pull
git submodule init # adiciona o url à path do submodulo no .gitmodules
git submodule update # clona o repositorio do submodulo
```
- atualizar o repo SplashBeachgoer com o ultimo commit do submodulo:
```bash
cd path/to/SplashBeachgoer
git pull
git submodule update --remote
git add PulseiraSplash
git commit -m "Atualizar commit do submodulo"
git push
```


# Estrutura do projeto (INCOMPLETO)
```
├── docs/                 	 # Documentação utilizada
|  ├── ESP32/
│  ├── FreeRTOS/
│  ├── NEO-6M/
│  └── BLE_Assigned_Numbers.pdf  
├── include/                 # Headers adicionais
│   └── ...
├── components/              # Bibliotecas locais
│   ├── nimble_central_utils/
│   └── nimble_peripheral_utils/
├── src/                     # Código-fonte principal
│   ├── ble/
│   ├── gps/
│   ├── utils/
│   ├── main.c             
│   └── ...
├── test/                    # Testes de unidade ou de integração
│   ├── test_circular_buffer/ 
│   ├── test_neo6m_setup/
│   └── test_ubx/
├── .gitignore
├── CMakeLists.txt           # Arquivo de configuração do CMake
├── platformio.ini           # Arquivo de configuração principal do PlatformIO
├── sdkconfig.esp32-s3-wroom-1 		# Configuração do ESP32 
├── sdkconfig.esp32-s3-wroom-1-test # Configuração do ESP32 para o enviroment de teste
└── README.md                
```

# Configuração e Instalação
1. Instalar ESP-IDF: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html
2. Instalar PlatformIO: https://platformio.org/install

	Opcional (Linux): Configurar uma alias para ativar o ambiente virtual do PlatformIO automaticamente
	```bash
	echo "alias pio_env='source ~/.platformio/penv/bin/activate'" >> ~/.bashrc
	bash ~/.bashrc
	```

3. Configurar o ESP32 no menu de configuração (INCOMPLETO)
	```bash
	pio run -t menuconfig
	```
4. Configurar upload port e monitor port no arquivo `platformio.ini`
	- Para descobrir a porta no Linux:
		procurar o dispositivo:
		```bash
		lsusb
		```
		ou usar o platformio:
		```bash
		pio_env # ativar ambiente virtual do PlatformIO (opcional)
		pio device list
		```
	- Para descobrir a porta no Windows:
		procurar o dispositivo:
		```powershell
		[System.IO.Ports.SerialPort]::getportnames()
		```
		ou usar o platformio:
		```bash
		pio device list
		```
5. Selecionar o modelo do ESP32-S3-WROOM-1 N16R8 no PlatformIO
6. Fazer build do projeto no PlatformIO
7. Fazer upload do projeto no PlatformIO para o ESP32

# Funcionalidades

No serviço "Location and Navigation", a caracteristica "Location and Speed" transmite os seguintes dados:
- Latitude
- Longitude
- Altitude
- Velocidade
- Ano
- Mês
- Dia
- Hora
- Minuto
- Segundo

Estes dados são codificados num buffer de 20 bytes da seguinte forma (**estrutura ainda a ser considerada**):

| **Byte(s)** | **Tamanho (bytes)** | **Descrição**                     | **Exemplo**                     |
|-------------|----------------------|-----------------------------------|----------------------------------|
| `0`         | 1                    | Flags indicando os dados presentes | `LN_FLAG_LOCATION_PRESENT \| LN_FLAG_ELEVATION_PRESENT \| LN_FLAG_TIME_PRESENT \| LN_FLAG_SPEED_PRESENT` |
| `1-4`       | 4                    | Latitude (multiplicado por `1e7`) | `-235678901` (representa -23.5678901°) |
| `5-8`       | 4                    | Longitude (multiplicado por `1e7`) | `-467890123` (representa -46.7890123°) |
| `9-10`      | 2                    | Altitude (multiplicado por `10`)   | `1234` (representa 123.4 m)      |
| `11-12`     | 2                    | Velocidade (em m/s)                | `15` (representa 15 m/s)         |
| `13-14`     | 2                    | Ano (em formato `uint16_t`)        | `2025`                           |
| `15`        | 1                    | Mês                                | `4` (representa abril)           |
| `16`        | 1                    | Dia                                | `1`                              |
| `17`        | 1                    | Hora (ajustada para o fuso horário)| `14` (representa 14h)            |
| `18`        | 1                    | Minuto                             | `30`                             |
| `19`        | 1                    | Segundo                            | `45`                             |


## A ter em conta para a localização:

- Latitude: 1 grau ≈ 111,32 km
- Longitude: 1 grau ≈ 111,32×cos(latitude) **a definição depende da latitude - quanto mais perto do equador menor**
	- latitude 90º (polos): 1 grau ≈ 111,32×cos(90) = 0 km
	- latitude 0º (equador): 1 grau ≈ 111,32×cos(0) = 111,32 km

|  Variações    | Precisão (da Latitude/minima da Longitude) em disntância |
| ------------- | ------------------------- 			|
| **0,1º**      | ~ 11,1 km                          	|
| **0,01º**     | ~ 1,11 km                 			|
| **0,001º**    | ~ 111 m                   			|
| **0,0001º**   | ~ 11,1 m                  			|
| **0,00001º**  | ~ 1,11 m                  			|
| **0,000001º** | ~ 0,111 m (~ 11,1 cm)     			|
| **0,0000001º**| ~ 0,0111 m (~ 1,11 cm)    			|
| **...**       | ...                       			|

**Conclusão**: A precisão da localização requirida para este dispositivo é de 0,000001 graus (ou 11,1cm de precisão minima). 

# Por fazer / considerar
- [ ] Tornar ble seguro
- [ ] Aumentar a precisão e exactidão do GPS (tanto em termos de programar o NEO-6M como o ESP32)
- [ ] Melhorar a gestão de energia (sleep mode)
- [ ] Optimizar o uso dos processadores e da memória
- [ ] Optimizar as antenas (maior alcance de sinal) (usar a partition phy para configurar a antena per-device em vez de por software)
- [ ] Aplicar Proximity Profile para calcular distância entre dispositivos

## Futuro fazer / coniderar
- [ ] Planear um pcb para a pulseira
- [ ] Encriptar o software
- [ ] Aplicar o BLE Mesh para aumentar o alcance do sinal
- [ ] Programar dispositivo para programar o NEO-6M (ou outro sensor que seja optado futuramente), para ter uma configuração fixa e não ter que estar a programa-lo sempre que se liga
- [ ] Adicionar à pulseira OTA para futuras atualizações ou criar um dispositivo para fazer upload de novas versões do firmware
- [ ] Adicionar à pulseira um SPIFFS ou FAT ou LittleFS ou MicroSD para guardar dados tanto para gerenciamento de dispositivos e firmware como para guardar dados de localização (tipo orange box)
- [ ] Adicionar sensores (ex: acelerometro, etc) e atuadores (ex: buzzer, led, etc) 
- [ ] Adicionar ao software da pulseira um sistema de gestão do dispositivo (ex: para ver a versão do firmware, para ver o estado da bateria (seria necessario saber préviamente o hardware da pulseira) , etc)