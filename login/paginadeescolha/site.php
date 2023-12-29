<?php

    @session_start();
    require_once "config.php";

    function logout()
    {
        session_unset();
        session_destroy();
        header("Location: index.php");
        exit();
    }

    if (!isset($_SESSION["loggedin"]) || $_SESSION["loggedin"] !== true) {
        header("Location: index.php");
        exit();
    }

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
    <link rel="stylesheet" href="paginadeescolha/pagina.css">
    
</head>
<body>
    
    <div class="container">
        <a href=" target=_blank "><img src="Mostrabot-é-um-projeto-da-Equipe-de-Robótica-da-Fundação-Liberato-realizadora-da-Mostratec-400x300.png" width="650" height="600">
        
    
        <p>O Mostrabot é o robô da Mostra Internacional de Ciência e Tecnologia (Mostratec), que será levado a <br>
            todos os interessados em aprender, de forma prática e à distância, como funciona de forma geral o <br>
            desenvolvimento de um robô.</p>
     </div>
</body>
</html>