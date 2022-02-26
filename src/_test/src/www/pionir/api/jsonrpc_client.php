<?php
/**
 * example to test www/pionir/api/jsonrpc.php
 */

namespace Example;
require __DIR__ . '/vendor/autoload.php';
use Datto\JsonRpc\Http\Client;
use Datto\JsonRpc\Http\HttpException;
use Datto\JsonRpc\Http\HttpResponse;
use Datto\JsonRpc\Response;
use ErrorException;

$client = new Client('http://jenblipart02/pionir/api/jsonrpc.php');

$client->query(1, 'add', array(1, 2));

$reply = $client->send();

echo "RESAULT:";
var_dump($reply);

