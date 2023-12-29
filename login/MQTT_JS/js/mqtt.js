$(document).ready(function(){
    // Create a client instance
 var client = new Paho.MQTT.Client('test.mosquitto.org', 8080,"clientId" + new Date().getTime);

// set callback handlers
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

// connect the client
client.connect({onSuccess:onConnect});


// called when the client connects
function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("Conectado ao MQTT Broker");
  client.subscribe("aula_mqtt/web");
  message = new Paho.MQTT.Message("Hello");
  message.destinationName = "World";
  client.send(message);
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("Conexão perdida:"+responseObject.errorMessage);
  }
}

// called when a message arrives
function onMessageArrived(message) {
  console.log("Mensagem recebida:"+message.payloadString);
 }

  function publishMessage(topic, message){
    var message = new Paho.MQTT.Message(message);
    message.destinationName= topic;
    client.send(message);
  } 

  $('#publicar').click(function(){
    var topico = $('#topico').val();
    var mensagem = $('#mensagem').val();

    publishMessage(topico,mensagem);
  });


});



