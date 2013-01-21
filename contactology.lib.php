<?php // $Id: //sms/modules/morris/contactology/6/v2012.1/contactology.lib.php#4 $

class ContactologyAPI extends Contactology {
  // Override the makeCall function provided by Contactology, then we may set a timeout for curl HTTP requests.
  protected function makeCall( $args ) {
    $ch = curl_init( $this->url );
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query( $args, null, "&" ) );
    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_USERAGENT, "Contactology PHP Wrapper {$this->version}" );
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, variable_get('contactology_connect_timeout', 2));
    curl_setopt($ch, CURLOPT_TIMEOUT, variable_get('contactology_response_timeout', 3));
    $json = curl_exec( $ch );
    if ( $json === false && $curl_error = curl_error($ch)) {
      throw new ContactologyException("Error: $curl_error");
    }
    $data = json_decode( $json, true );
    // The Contactology REST API is inconsistent with its error reporting, so it is tough to capture all errors.
    if ($data === false ) {
      throw new ContactologyException("Unspecified error from Contactology.");
    }
    if (isset( $data['result'] ) && $data['result'] == "error") {
      throw new ContactologyException( "API Error: {$data['message']}", $data['code']);
    }
    if ($data['errors'][0]['_msg_']) {
      throw new ContactologyException("Error: {$data['errors'][0]['_msg_']}", $data['errors'][0]['_err_']);
    }
    if (isset($data['success']) && !$data['success']) {
      throw new ContactologyException("Uncaught error from Contactology.");
    }

    return $data;
  }
  
}

class ContactologyException extends Exception {
  public function __construct($message = null, $code = 0) {
    $log_errors = variable_get('contactology_enable_logging', 0);
    if ($log_errors) {
      watchdog('contactology', $message, NULL, WATCHDOG_WARNING);
    }
    parent::__construct($message, $code);
  }
}