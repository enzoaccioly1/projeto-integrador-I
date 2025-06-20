# 🌧️ | ESP RainMaker

ESP RainMaker é um sistema IoT de baixo custo com o backend em nuvem construído utilizando a arquitetura 'serverless' da AWS. 'Serverless' não significa que não há servidor, mas sim que a gestão da infraestrutura de hardware e software é oculta, possibilitando o usuário desenvolver sistemas IoT amplamente customizaveis com uma quantidade mínima de código.

---

## 📡 | Comunicação

<kbd>
  <img src = "img/rainmaker.png">
</p>
  <p align = center>
    Funcionamento da comunicação com ESP RainMaker
  </p>
</kbd>

---

1 - AIoT Device: O dispositivo AIoT (ex.: lâmpada inteligente, ar-condicionado) conecta-se à nuvem por meio de redes sem fio, enviando e recebendo dados.

2 - AWS Cloud Backend:

  O backend em nuvem é composto por:

  - AWS IoT MQTT Broker: Responsável por gerenciar a comunicação via protocolo MQTT, garantindo troca de mensagens segura e eficiente entre o dispositivo e os serviços de aplicação.

  - Core Application Services: Conjunto central de serviços que processa dados, gerencia lógica de negócios e coordena interações entre dispositivos, usuários e aplicativos.

  - API: Interface de programação de aplicações que permite a integração com dashboards, assistentes de voz, apps móveis e outras aplicações externas.

  - Notification: Sistema de envio de notificações aos usuários, informando eventos e estados dos dispositivos.

  - Web Hooks e OAuth2 Plug: Mecanismos para integração com aplicações externas e autenticação segura de usuários.

3 - Integrações e Interfaces:

  - Voice Assistant: Controle e monitoramento dos dispositivos por meio de assistentes de voz (ex.: Alexa, Google Assistant).

  - Dashboard: Interface web para visualização e gestão dos dispositivos conectados.

  - Mobile APP: Aplicativo móvel para configuração, controle e monitoramento remoto.

  - Applications e User Management: Aplicações externas e sistema de gerenciamento de usuários, permitindo administração de permissões, autenticação e integração com outros sistemas.


---

## ⚠️ | Observações 
- O limite de envio de mensagens no plano grátis é de 25000.
- O número recomendado de dispositivos é de 5.
