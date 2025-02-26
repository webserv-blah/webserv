#!/usr/bin/php-cgi
<?php
// Set content type to HTML
header("Content-type: text/html");

// Get the name parameter from the query string or use a default
$name = isset($_GET['name']) ? htmlspecialchars($_GET['name']) : 'World';

// Output HTML content
echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<head>\n";
echo "    <title>Hello from PHP!</title>\n";
echo "    <style>\n";
echo "        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n";
echo "        h1 { color: #336699; }\n";
echo "        p { color: #444; }\n";
echo "        .container { max-width: 600px; margin: 0 auto; padding: 20px; border: 1px solid #ddd; border-radius: 5px; }\n";
echo "        .time { font-size: 0.9em; color: #777; margin-top: 30px; }\n";
echo "    </style>\n";
echo "</head>\n";
echo "<body>\n";
echo "    <div class='container'>\n";
echo "        <h1>Hello, $name!</h1>\n";
echo "        <p>This is a simple PHP CGI script running on the custom webserver.</p>\n";
echo "        <p>Try adding a name parameter to the URL: <code>?name=YourName</code></p>\n";
echo "        <p class='time'>Current server time: " . date('Y-m-d H:i:s') . "</p>\n";
echo "    </div>\n";
echo "</body>\n";
echo "</html>\n";
?>