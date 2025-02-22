# Decimeter: Monitoramento Autônomo de Níveis de Ruído

O Decimeter é um projeto desenvolvido para monitorar níveis de ruído em ambientes de forma autônoma e em tempo real. Utilizando um sensor de som MAX4466, uma matriz de LEDs e um display OLED, o dispositivo permite que o usuário defina um limite de ruído e receba alertas visuais quando esse limite é ultrapassado. O projeto foi desenvolvido com a placa Raspberry Pi Pico W e programado em linguagem C, utilizando o SDK da Raspberry Pi.

## Funcionalidades Principais
- Monitoramento de Ruído: Captura níveis de ruído em tempo real utilizando o sensor MAX4466.
- Alertas Visuais: Aciona uma matriz de LEDs quando o limite de ruído configurado é atingido.
- Interface Gráfica: Exibe informações no display OLED, incluindo o valor atual de dB, uma barra de progresso e um menu interativo.
- Configuração de Limites: Permite ao usuário definir um limite de ruído em dB.
- Operação Autônoma: Funciona sem necessidade de intervenção humana constante.

## Como Rodar o Projeto
- Software:
    - Visual Studio Code
    - Extensão Raspberry Pi Pico Project
    - SDK da Raspberry Pi Pico (v2.1.1 ou superior)
    - CMake

### Clonagem do Projeto
- Clone o repositório do projeto:
```
    git clone https://github.com/GabrielShiva/final-project-embarcatech.git
```
- Abra o VsCode e clique na extensão e escolha a opção de `importar projeto`. Selecione a pasta e espere a extensão gerar os arquivos necessários para o correto funcionamento do projeto na placa. Após isso, o projeto está pronto para ser manipulado.

### Compilação e Upload
- Compile o projeto:
    -  No Visual Studio Code, clique no botão `compilar` na parte inferior da IDE.
    - Conecte a Raspberry Pi Pico W ao computador via USB.
    - Carregue o firmware no microcontrolador:
        - Copie o arquivo .uf2 gerado na pasta build para a unidade de armazenamento da Pico.

### Execução
    - Após o upload do firmware, o dispositivo iniciará automaticamente.
    - Utilize os botões para navegar no menu e configurar o limite de ruído.
    - O display OLED exibirá o valor atual de dB e a barra de progresso.
    - Quando o limite de ruído for atingido, a matriz de LEDs será acionada.

## Vídeo de Apresentação

Para uma demonstração visual do funcionamento do projeto, assista ao vídeo [clicando aqui](https://youtu.be/d9DqBkpke1U)