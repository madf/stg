<?php

// Call helper

function __call($method, $params = null)
{
    $request = xmlrpc_encode_request($method, $params, array('escaping' => 'markup', 'encoding' => 'utf-8'));
    $context = stream_context_create(
            array('http' => array(
                        'method' => 'POST',
                        'header' => 'Content-Type: text/xml',
                        'content' => $request)));

    $file = file_get_contents('http://localhost:8080/RPC2', false, $context);
    $response = xmlrpc_decode($file);

    if (is_array($response) && xmlrpc_is_fault($response)) {
        trigger_error("xmlrpc: {$response['faultString']} ({$response['faultCode']})");
    }

    return $response;
}

// Usage samples

print_r(__call('stargazer.info'));
$data = __call('stargazer.login', array('admin', '123456'));
if (isset($data['cookie'])) {
    $cookie = $data['cookie'];
    print_r($data);
    print_r(__call('stargazer.get_tariffs', array($cookie)));
    print_r(__call('stargazer.logout', array($data['cookie'])));
}

?>

