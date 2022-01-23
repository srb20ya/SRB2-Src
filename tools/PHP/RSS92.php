<?php
header("Content-type: text/xml; charset=ISO-8859-1");
echo '<?xml version="1.0" encoding="ISO-8859-1" ?>';
echo "\n";
echo '<rss version="0.92">';
echo "\n";
echo '  <channel>';
echo "\n";
echo '    <title>SRB2 Master Server RSS Feed</title>';
echo "\n";
echo '    <description>Playing around with RSS</description>';
echo "\n";
echo '    <link>http://srb2.servegame.org/srb2ms_status.php</link>';
echo "\n";
echo '    <language>en-us</language>';
echo "\n";
        $fd = fsockopen("srb2.servegame.org", 28910, $errno, $errstr, 5);
	if ($fd)
	{
		$buff = "000012380000";
		fwrite($fd, $buff);
		$content=fgets($fd, 13); // skip 13 first bytes
		while (1)
		{
			$content=fgets($fd, 1024);
			echo "$content";
			if (feof($fd)) break;
		}
		fclose($fd);
	}
	else
	{
		echo "<item><title>No master server</title><description>The master server is not running</description></item>";
	}
?>
  </channel>
</rss>