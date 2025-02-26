#!/usr/bin/php-cgi
<?php
// Set content type to HTML
header("Content-type: text/html");

// Start HTML document
echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<head>\n";
echo "    <title>PHP CGI Example</title>\n";
echo "    <style>\n";
echo "        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }\n";
echo "        h1 { color: #336699; }\n";
echo "        h2 { color: #444; margin-top: 20px; }\n";
echo "        pre { background: #f4f4f4; padding: 10px; border-radius: 5px; }\n";
echo "        .data-section { margin-bottom: 30px; }\n";
echo "        table { border-collapse: collapse; width: 100%; }\n";
echo "        table, th, td { border: 1px solid #ddd; }\n";
echo "        th, td { padding: 8px; text-align: left; }\n";
echo "        th { background-color: #f2f2f2; }\n";
echo "    </style>\n";
echo "</head>\n";
echo "<body>\n";
echo "    <h1>PHP CGI Example</h1>\n";

// Display CGI environment variables
echo "    <div class='data-section'>\n";
echo "        <h2>CGI Environment Variables</h2>\n";
echo "        <table>\n";
echo "            <tr><th>Variable</th><th>Value</th></tr>\n";

// List of CGI environment variables we want to display
$env_vars = array(
    'GATEWAY_INTERFACE',
    'SERVER_PROTOCOL',
    'SERVER_SOFTWARE',
    'SERVER_NAME',
    'SERVER_PORT',
    'REQUEST_METHOD',
    'PATH_INFO',
    'SCRIPT_NAME',
    'QUERY_STRING',
    'REMOTE_ADDR',
    'CONTENT_TYPE',
    'CONTENT_LENGTH'
);

// Display each environment variable
foreach ($env_vars as $var) {
    if (isset($_SERVER[$var])) {
        echo "            <tr><td>" . htmlspecialchars($var) . "</td><td>" . htmlspecialchars($_SERVER[$var]) . "</td></tr>\n";
    }
}
echo "        </table>\n";
echo "    </div>\n";

// Process and display GET parameters
if (!empty($_GET)) {
    echo "    <div class='data-section'>\n";
    echo "        <h2>GET Parameters</h2>\n";
    echo "        <table>\n";
    echo "            <tr><th>Parameter</th><th>Value</th></tr>\n";
    foreach ($_GET as $key => $value) {
        echo "            <tr><td>" . htmlspecialchars($key) . "</td><td>" . htmlspecialchars($value) . "</td></tr>\n";
    }
    echo "        </table>\n";
    echo "    </div>\n";
}

// Process and display POST parameters
if (!empty($_POST)) {
    echo "    <div class='data-section'>\n";
    echo "        <h2>POST Parameters</h2>\n";
    echo "        <table>\n";
    echo "            <tr><th>Parameter</th><th>Value</th></tr>\n";
    foreach ($_POST as $key => $value) {
        echo "            <tr><td>" . htmlspecialchars($key) . "</td><td>" . htmlspecialchars($value) . "</td></tr>\n";
    }
    echo "        </table>\n";
    echo "    </div>\n";
}

// Display request headers
echo "    <div class='data-section'>\n";
echo "        <h2>HTTP Request Headers</h2>\n";
echo "        <table>\n";
echo "            <tr><th>Header</th><th>Value</th></tr>\n";
foreach (getallheaders() as $name => $value) {
    echo "            <tr><td>" . htmlspecialchars($name) . "</td><td>" . htmlspecialchars($value) . "</td></tr>\n";
}
echo "        </table>\n";
echo "    </div>\n";

// End HTML document
echo "</body>\n";
echo "</html>\n";
?>