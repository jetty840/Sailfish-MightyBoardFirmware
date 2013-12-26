BEGIN { FS = ":" }
{
	gsub("[MSP]","");
	if ( $2 ) {
	   { print $2; exit }
	}
	else {
	   { print $1; exit }
	}
}
