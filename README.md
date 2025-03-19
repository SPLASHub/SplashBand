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
├── include/                 # Headers adicionais
│   └── ...
├── lib/                     # Bibliotecas locais
│   └── ...
├── src/                     # Código-fonte principal
│   ├── main.cpp             
│   └── ...
├── test/                    # Testes de unidade ou de integração
│   └── ...
├── .gitignore
├── CMakeLists.txt           # Arquivo de configuração do CMake
├── platformio.ini           # Arquivo de configuração principal do PlatformIO
├── sdkconfig.esp32-s3-wroom-1 # Configuração do ESP32
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
