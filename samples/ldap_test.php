#!/usr/bin/php
<?php
require('../src//ldap.inc');

echo "    User: ";
$user = trim(fgets(STDIN));

echo "Password: ";
system('stty -echo');
$password = trim(fgets(STDIN));
system('stty echo');
// add a new line since the users CR didn't echo
echo "\n";

$ldap = new ldap_auth('ldap.ini');
if ($ldap->auth($user,
		$password)) {
  echo "Success\n";
} else {
  echo "Login Failed\n";
}
unset($ldap);
?>
