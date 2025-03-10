# Monitoramento de Temperatura para Forno Industrial

## Alunos:
- Alyson Matheus Vale
- Pedro Savio

## Introdução

A manutenção preditiva é um elemento essencial na indústria moderna, permitindo a detecção antecipada de falhas e otimização da operação de equipamentos. Este projeto tem como objetivo monitorar a temperatura de um forno industrial em tempo real, prevenindo superaquecimentos e garantindo maior eficiência operacional.

O sistema utiliza sensores para capturar a temperatura do forno e um display OLED para exibir os valores coletados. Além disso, um LED de alerta é ativado sempre que a temperatura ultrapassa um limite predefinido, sendo desativado quando a temperatura retorna a níveis seguros.

## Descrição do Sistema

O projeto é composto pelos seguintes componentes principais:

- **DS18B20**: Sensor de temperatura digital de alta precisão, responsável por medir a temperatura do forno.
- **SSD1306**: Display OLED para exibição da temperatura em tempo real.
- **LED de Alerta**: Dispositivo luminoso que acende caso a temperatura ultrapasse o limite predefinido e apaga quando retorna ao normal.
- **SSR**: Um relé de estado sólido (SSR) é utilizado através de uma GPIO conectada ao LED, permitindo o acionamento do alerta.

A integração desses componentes possibilita um monitoramento contínuo e uma resposta imediata caso a temperatura atinja valores críticos.

## Aplicações e Benefícios

Este projeto tem diversas aplicações industriais, incluindo:

- **Forno Industrial**: Monitoramento da temperatura para evitar superaquecimentos e garantir a eficiência do processo produtivo.
- **Setor Automotivo**: Controle térmico em processos de fundição e tratamento térmico de peças.
- **Indústria Alimentícia**: Monitoramento de temperatura em fornos de panificação e outros equipamentos térmicos.

Os principais benefícios do projeto incluem:

- **Redução de Riscos**: Evita danos causados por superaquecimento em equipamentos industriais.
- **Maior Eficiência Energética**: Permite um controle mais preciso do uso de energia no forno.
- **Aumento da Segurança**: Monitoramento contínuo reduz o risco de falhas catastróficas.

## Implementação

O projeto foi desenvolvido utilizando um microcontrolador ESP com conexão Wi-Fi, permitindo futuras expansões para monitoramento remoto e alertas via rede. A medição da temperatura ocorre periodicamente, e o LED de alerta é acionado sempre que os valores ultrapassam um limite predefinido.

### Fluxo de Funcionamento

1. O **sensor DS18B20** lê a temperatura do forno.
2. O **display SSD1306** exibe o valor da temperatura em tempo real.
3. Se a temperatura ultrapassar o limite, o **LED de alerta** acende.
4. Quando a temperatura retorna ao nível seguro, o **LED de alerta** apaga.

### Configuração de Alerta

- Temperatura máxima permitida: **Definida pelo usuário**
- LED acende quando: **Temperatura > limite predefinido**
- LED apaga quando: **Temperatura ≤ limite predefinido**

## Link para Vídeo e Imagens

Para facilitar a visualização do funcionamento do sistema, disponibilizamos os seguintes links:

📹 **Vídeo Demonstrativo**: [https://youtube.com/shorts/Vq9VWbWFRvo]

🖼️ **Imagens do Projeto**: [https://github.com/alysonvale/sistemasEmbarcados/blob/main/projeto.jpeg]

## Conclusão

O monitoramento contínuo da temperatura do forno industrial é essencial para evitar falhas, reduzir desperdícios energéticos e aumentar a segurança. O projeto propõe uma solução acessível e eficiente, utilizando sensores de temperatura e um sistema de alerta visual.

Futuramente, o sistema pode ser expandido com integração de conectividade remota, permitindo que alertas sejam enviados para dispositivos móveis e sistemas de controle automatizado.

---

**Autores:**  
Alyson Matheus Vale | Pedro Savio  
