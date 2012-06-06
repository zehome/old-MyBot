<?php

class MyBot
{
  var $port = 6161;
  var $host;

  var $sock = -1;

  function MyBot($host, $port = 6161)
  {
    $this->host = $host;
    $this->port = $port;
  }

  function __disconnect()
  {
    @socket_close($this->sock);
    $this->sock = -1;
  }

  function __connect()
  {
    $this->sock = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if (! @socket_connect($this->sock, $this->host, $this->port))
    {
      /* Error connecting... */
      $this->sock = -1;
      return -1;
    }
  }

  function getNbNicks($channel)
  {
    if ($this->__connect() == -1)
      return -1;
    
    @socket_write($this->sock, "HOWMANY ".$channel."\n");
    
    $i = 0;
    $buf = "";
    while (($r = @socket_read($this->sock, 1)) != False)
    {
      if ($r == '\n')
        break;
      $buf .= $r;
    }

    if (strstr($buf, "ERROR"))
    {
      echo "Erreur: ".$buf."\n";
      return 0;
    }

    $array = split(" ",$buf);
    $channel = $array[0];
    $nicks = $array[1];

    $this->__disconnect();
    return $nicks-1;
  }

  function getNicks($channel)
  {
    if ($this->__connect() == -1)
      return False;
    
    @socket_write($this->sock, "NICKS ".$channel."\n");
    
    $i = 0;
    $buf = "";
    while (($r = @socket_read($this->sock, 1)) != False)
    {
      if ($r == "\n")
        break;
      $buf .= $r;
    }

    if (strstr($buf, "ERROR"))
    {
      echo "Erreur: ".$buf."\n";
      return False;
    }

    $array = split(" ",$buf, 2);
    $channel = $array[0];
    $nicks = $array[1];
    $this->__disconnect();
    return $nicks;
  }
}

$mybot = new MyBot("192.168.1.11", 6161);
$resultat = $mybot->getNbNicks("#test");
echo "Nombre de personnes: ".$resultat."\n";
$resultat = $mybot->getNicks("#test");
echo "Personnes: ".$resultat."\n";

?>
