<?php

$width = 100;
$height = 80;

$name = "putty1_01.log";

$ascii_content = file_get_contents($name);

for ($y = 0; $y < $height; $y++)
	for ($x = 0; $x < $width;  $x++)
	{
		$content[$x][$y] = hexdec(substr($ascii_content, $x * 7 + 2 + ($y * ($width + 1) * 7), 4)) / 2;
		//echo substr($ascii_content, $x * 7 + 2, 4)." ";
		//echo $content[$x][$y]." ";
	}

//exit;

header ('Content-Type: image/png');
$im = @imagecreatetruecolor($width, $height) or die('Cannot Initialize new GD image stream');

for ($y = 0; $y < $height; $y++)
	for ($x = 0; $x < $width;  $x++)
	{
		$col = imagecolorallocate ($im, $content[$x][$y], 0, 0);
		imagesetpixel ($im, $x, $y, $col);
	}

imagepng($im);
imagedestroy($im);

?>