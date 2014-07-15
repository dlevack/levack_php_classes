#!/usr/bin/php
<?php
require('../src/ldap.php');

echo "Password: ";
system('stty -echo');
$password = trim(fgets(STDIN));
system('stty echo');
// add a new line since the users CR didn't echo
echo "\n";

/**
 * I have noticed an issue with AD binding successfully with a blank pass.
 * The following code fixes this by changeing a blank pass to '!!!!'
 */
if ($password == '') {
  $password = '!!!!';
}

// Example of authenticating against AD
$ldap = new ldap_auth('dc01.example.com',
		      'dc=example,dc=com');
$ldap->connect();
if ($ldap->bind('jdow',
                $password)) {
  echo "Success\n";
} else {
  echo "Login Failed\n";
}
unset($ldap);

// Example of authenticating against Open LDAP
$ldap = new ldap_auth('localhost',
		      'dc=example,dc=com',
		      2);
$ldap->connect();
if ($ldap->bind('jdow',
		$password)) {
  echo "Success\n";
} else {
  echo "Login Failed\n";
}
unset($ldap);
?>
