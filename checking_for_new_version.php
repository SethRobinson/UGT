<?php

$latestVersion = 75;
$prettyVersion = "0.75 Beta";
$downloadLink = "https://www.rtsoft.com/files/UniversalGameTranslator_Windows.zip";
$updateString = "A new version of UGT has been released.\r\nDownload it now? (don't forget to quit the app before copying over the old one)\r\nTo disable update checks, edit your config.txt file.";

if (isset($_GET["version"]))
{
	
}
echo 'latest_version|'.$latestVersion.'|'.$prettyVersion.'|'.$downloadLink.'|'.$updateString.'|'; 
	
exit();
?>
