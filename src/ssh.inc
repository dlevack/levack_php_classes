<?php
/**
 * Provides methods for utilizing ssh within php scripts.
 *
 * This is the ssh.php file. It will provide methods for
 * utilizing ssh within php scripts.
 *
 * @author Dan Levack <dan@levack.net>
 * @version 1.0
 */

/**
 * Provides methods for utilizing ssh within php scripts.
 */

class ssh_conn {
  
  /**
   * @var string $HOST Host to connect to
   */
  public $HOST;
  
  /**
   * @var string $USER User to use when connecting
   */
  public $USER;
  
  /**
   * @var string $KEY Key to use when connecting
   */
  public $KEY;
  
  /**
   * @var string $OPTIONS Options to use when connecting
   */
  public $OPTIONS;
  
  /**
   * Conctructor method, called when class is called.
   *
   * The constructor will set the HOST, USER, KEY and OPTIONS variables
   *
   * @param string $host    Host to connect to
   * @param string $user    User to use when connecting
   * @param string $key     Key to use when connecting
   * @param int    $timeout Timeout For connection
   */
  public function __construct($host    = '',
			      $user    = '',
			      $key     = '',
			      $timeout = 2) {
    $this->set_host($host);
    $this->set_user($user);
    $this->set_key($key);
    $this->set_timeout($timeout);
    return;
  }
  
  /**
   * Returns the output of the command as an array.
   *
   * @param  string $cmd    Command to be run
   * @return array  $return Array of output from command
   */
  public function cmd($cmd = '') {
    // Start building SSH command and add timeout option
    $command = 'ssh '.$this->OPTIONS.' ';
    
    // Check to see if we were provided an SSH key and if so use it
    if ($this->KEY != '') {
      $command .= '-i '.$this->KEY.' ';
    }
    
    // Check to see if we were provided a user of the connection
    // and if so use it
    if ($this->USER != '') {
      $command .= $this->USER.'@';
    }
    
    // Add the host to the command
    $command .= $this->HOST." '";
    
    // Add the command we want to run to the command
    $command .= $cmd;
    
    // Redirect STDOUT to STDIN so that any errors will be returned
    // in the array
    $command .= "' 2>&1";
    
    $return   = array();
    
    // Run the command and set its output in the $return array
    exec($command,
	 $return);
    
    return($return);
  }
  
  /**
   * Returns an array of data showing the disk space information
   * provided by the 'df -h' (Human Readable) command.
   *
   * @return array $df Disk space information
   */
  public function get_df() {
    $df      = array();
    $headers = array();
    
    // Run the 'df -h' command on remote host
    $data    = $this->cmd('df -h');
    
    // Build an array of the column headings
    foreach (explode(' ', $data[0]) as $k=>$v) {
      if ($v != '') {
	if ($v == 'Mounted') {
	  $v = 'Mounted on';
	}
	if ($v != 'on') {
	  $headers[] = $v;
	}
      }
    }
    
    $line = 0;
    
    // Parse the disk space information skiping the first line since
    // we already proccesed that line for the headings
    for ($x = 1; $x < count($data);$x++) {
      $num = 0;
      
      // Check to see if this line of output contains a % sign
      // If it does not that means that the data is contained in two
      // lines of output
      if (!preg_match('/%/',$data[$x])) {
	
	// Add the filesystem to the array
	$df[$line][$headers[$num]] = $data[$x];
	$x++;
	$num++;
	
	// Break the line up on spaces and populate the array with the data
	foreach (explode(' ',$data[$x]) as $k=>$v) {
	  if ($v != '') {
	    $df[$line][$headers[$num]] = $v;
	    $num++;
	  }
	}
      }  else {
	
	// Break the line up on spaces and populate the array with the data
	foreach (explode(' ',$data[$x]) as $k=>$v) {
	  if ($v != '') {
	    $df[$line][$headers[$num]] = $v;
	    $num++;
	  }
	}
      }
      $line++;
    }
    
    unset($line);
    unset($data);
    unset($headers);
    return($df);
  }
  
  /**
   * Set the value of the host to SSH to.
   * 
   * @param string $host Host to SSH to
   */
  public function set_host($host = '') {
    $this->HOST = $host;
    return;
  }
  
  /**
   * Set the value of the user to use for the SSH connection.
   * 
   * @param string $user User to use when connecting
   */
  public function set_user($user = '') {
    $this->USER = $user;
    return;
  }
  
  /**
   * Set the value of the SSH key to use for the SSH connection.
   * 
   * @param string $key SSH key to use when connecting
   */
  public function set_key($key = '') {
    $this->KEY = $key;
    return;
  }
  
  /**
   * Set the value of the timeout to use for the SSH connection.
   *
   * @param int $timeout Timeout value to use when connecting
   */
  public function set_timeout($timeout = 2) {
    $this->OPTIONS = '-o ConnectTimeout='.$timeout;
    return;
  }
}
?>
