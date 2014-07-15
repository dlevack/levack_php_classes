<?php
/**
 * Provides Authentication against LDAP/AD servers
 * 
 * This is the ldap.php file. It will contain classes for LDAP
 * authentication.
 * 
 * @author Dan Levack <dan@levack.net>
 * @version 1.0
 */

/**
 * Provides authentication through Active Directory
 */

class ldap_auth {
  
  /**
   * @var string $HOST LDAP Host to connect to (dc01.example.com)
   */
  public $HOST;
  
  /**
   * @var int $TYPE 1 for AD or 2 for OpenLDAP
   */
  public $TYPE = 1;
  
  /**
   * @var string $DOMAIN The domain of the server AD (example) OpenLDAP (dc=example,dc=com)
   */
  public $DOMAIN;
  
  /**
   * @var handler $CONN LDAP connection handler
   */
  public $CONN;
  
  /**
   * @var handler $BIND LDAP bind handler
   */
  public $BIND;
  
  /**
   * Conctructor method, called when class is called.
   *
   * The constructor will set the HOST variable, connect to the
   * host and set a few LDAP options.
   * 
   * @param string $host   Host to connect to (dc01.example.com)
   * @param string $domain Domain for server (dc=example,dc=com)
   * @param int    $type   Type of LDAP server 1 = AD 2 = OpenLDAP
   */
  public function __construct($host   = '',
			      $domain = '',
			      $type   = 1) {
    $this->HOST   = $host;
    $this->DOMAIN = $domain;
    $this->TYPE   = $type;
    $this->connect();
    ldap_set_option($this->CONN,
		    LDAP_OPT_PROTOCOL_VERSION,
		    3);
    ldap_set_option($this->CONN,
		    LDAP_OPT_REFERRALS,
		    0);
    return;
  }
  
  /**
   * Connect to the LDAP host
   */
  public function connect() {
    $this->CONN = ldap_connect($this->HOST);
    return;
  }
  
  /**
   * Bind to the host using supplied cradentials
   *
   * @param string $user User name to bind with (jdow)
   * @param string $pass Password to bind with (password)
   * @return bool        TRUE if authenticated FALSE if not
   */
  public function bind($user = '',
		       $pass = '') {
    $this->BIND = @ldap_bind($this->CONN,
                             $this->get_dn_of_user($user),
                             $pass);
    if ($this->BIND) {
      return TRUE;
    } else {
      return FALSE;
    }
    return;
  }
  
  /**
   * Returns the DN of the user
   * 
   * @param string $user uid of the user
   * @return string $dn DN of the user
   */
  public function get_dn_of_user($user = '') {
    $dn     = '';
    if ($this->TYPE == 1) {
      $filter = "(sAMAccountName=".$user.")";
    } else {
      $filter = "(uid=".$user.")";
    }
    $justthese = array("dn");
    
    $sr = ldap_search($this->CONN,
		      $this->DOMAIN,
		      $filter,
		      $justthese);
    $info = ldap_get_entries($this->CONN,
			     $sr);
    $dn   = $info[0]['dn'];
    unset($justthese);
    unset($filter);
    return($dn);
  }
  
  /**
   * Deconstructor method, called when class is closed.
   * 
   * The destructor will close the ldap connection.
   */
  public function __destruct() {
    @ldap_close($this->CONN);
  }
}
?>
