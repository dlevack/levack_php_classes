<?php
/**
 * Provides Authentication against LDAP/AD servers
 * 
 * This is the ldap.php file. It will contain classes for LDAP
 * authentication.
 * 
 * @author Dan Levack <dan@levack.net>
 * @version 1.1
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
   * @var string $BASE_DN The base DN for the server
   */
  public $BASE_DN;
  
  /**
   * @var string $USERS_DN The user DN for the server
   */
  public $USERS_DN;
  
  /**
   * @var string $GROUPS_DN The group DN for the server
   */
  public $GROUPS_DN;
  
  /**
   * @var handler $CONN LDAP connection handler
   */
  public $CONN;
  
  /**
   * @var handler $BIND LDAP bind handler
   */
  public $BIND;
  
  /**
   * @var string $BIND_USER User to use to bind
   */
  private $BIND_USER;
  
  /**
   * @var string $BIND_PASS Password to use to bind
   */
  private $BIND_PASS;
  
  /**
   * Conctructor method, called when class is called.
   *
   * The constructor will set the HOST variable, connect to the
   * host and set a few LDAP options.
   * 
   * @param string $ini Path to INI file
   */
  public function __construct($ini = '') {
    require_once('ini_files.inc');
  
    $config          = new ini_files($ini);
    $this->HOST      = $config->CONFIG['host'];
    $this->BASE_DN   = $config->CONFIG['base_dn'];
    $this->USERS_DN  = $config->CONFIG['users_dn'].','.$this->BASE_DN;
    $this->GROUPS_DN = $config->CONFIG['groups_dn'].','.$this->BASE_DN;
    $this->TYPE      = $config->CONFIG['type'];
    $this->BIND_USER = $config->CONFIG['bind_user'];
    $this->BIND_PASS = $config->CONFIG['bind_pass'];
    unset($config);
    $this->connect();
    ldap_set_option($this->CONN,
		    LDAP_OPT_PROTOCOL_VERSION,
		    3);
    ldap_set_option($this->CONN,
		    LDAP_OPT_REFERRALS,
		    0);
    $this->bind();
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
   * @return bool TRUE if authenticated FALSE if not
   */
  public function bind() {
    $this->BIND = @ldap_bind($this->CONN,
                             $this->BIND_USER,
                             $this->BIND_PASS);
    if ($this->BIND) {
      return TRUE;
    } else {
      return FALSE;
    }
    return;
  }
  
  /**
   * Returns TRUE if authenticated or FALSE if failed
   *
   * @param  string $user Username
   * @param  string $pass password
   * @return bool         TRUE if authenticated successfully or FALSE if not
   */
  public function auth($user = '',
		       $pass = '') {
    if ($user == '') {
      return FALSE;
    }
    if ($pass == '') {
      return FALSE;
    }
    $auth = TRUE;
    @ldap_bind($this->CONN,
	       $this->get_dn_of_user($user),
	       $pass) or $auth = FALSE;
    if ($auth) {
      return TRUE;
    } else {
      return FALSE;
    }
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
		      $this->USERS_DN,
		      $filter,
		      $justthese);
    $info = ldap_get_entries($this->CONN,
			     $sr);
    if (isset($info[0]['dn'])) {
      $dn   = $info[0]['dn'];
    }
    unset($justthese);
    unset($filter);
    return($dn);
  }
  
  /**
   * Returns array of user information
   * 
   * @param  string $user User name
   * @return array  $info Array of user information
   */
  public function get_user_info($user) {
    $info = array();
    if ($this->TYPE == 1) {
      $filter    = "(sAMAccountName=".$user.")";
      $justthese = array("dn",
			 "cn",
			 "sAMAccountName",
			 "mail",
			 "memberof");
    } else {
      $filter    = "(uid=".$user.")";
      $justthese = array("dn",
                         "cn",
			 "uid",
                         "mail",
                         "memberof");
    }
    $sr = ldap_search($this->CONN,
                      $this->USERS_DN,
                      $filter,
    		      $justthese);
    $data = ldap_get_entries($this->CONN,
                             $sr);
    if (isset($data[0]['dn'])) {
      $info['dn'] = $data[0]['dn'];
    }
    if (isset($data[0]['cn'])) {
      $info['cn'] = $data[0]['cn'][0];
    }
    if ($this->TYPE == 1) {
      if (isset($data[0]['dn'])) {
        $info['samaccountname'] = $data[0]['samaccountname'][0];
      }
    } else {
      if (isset($data[0]['uid'])) {
	$info['uid'] = $data[0]['uid'][0];
      }
    }
    if (isset($data[0]['mail'])) {
      foreach ($data[0]['mail'] as $k=>$v) {
	if (preg_match('/@/',
		       $v)) {
	  $info['mail'][] = $v;
	}
      }
    }
    if ($this->TYPE == 1) {
      if (isset($data[0]['memberof'])) {
	foreach ($data[0]['memberof'] as $k=>$v) {
	  if (preg_match('/=/',
			 $v)) {
	    $info['memberof'][] = $v;
	  }
	}
      }
    } else {
      foreach ($this->get_group_list() as $k=>$v) {
	if ($this->is_user_member($info['dn'],
				  $v)) {
	  $info['memberof'][] = 'cn='.$v.','.$this->GROUPS_DN;
	}
      }
    }
    
    unset($justthese);
    unset($filter);
    return($info);
  }
  
  /**
   * Returns array of group names
   * 
   * @return array $groups Array of group names
   */
  public function get_group_list() {
    $groups = array();
    $filter = "cn=*";
    $sr = ldap_search($this->CONN,
                      $this->GROUPS_DN,
                      $filter);
    $info = ldap_get_entries($this->CONN,
                             $sr);
    foreach ($info as $k=>$v) {
      if (isset($v['cn'])) {
	$groups[] = $v['cn'][0];
      }
    }
    unset($filter);
    return($groups);
  }
  
  /**
   * Returns TRUE if user is a member of group and FALSE if not
   * 
   * @param  string $user_dn  Username to check
   * @param  string $group_cn CN of group to check
   * @return bool             TRUE if user is a member of group and FALSE if not
   */
  public function is_user_member($user_dn,
				 $group_cn) {
    $filter = "cn=".$group_cn;
    $justthese = array("member");
    
    $sr = ldap_search($this->CONN,
                      $this->GROUPS_DN,
                      $filter,
		      $justthese);
    $info = ldap_get_entries($this->CONN,
                             $sr);
    
    foreach ($info[0]['member'] as $k=>$v) {
      if ($v == $user_dn) {
	unset($justthese);
	unset($filter);
	return TRUE;
      }
    }
    unset($justthese);
    unset($filter);
    return FALSE;
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
