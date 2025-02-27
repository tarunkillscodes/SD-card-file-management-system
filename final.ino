#include <WiFi.h>
#include <SD.h>
#include <WebServer.h>

// Wi-Fi credentials
const char* ssid = "Himmler2005";
const char* password = "tarun@2005";

// SD card CS pin
const int sdCardCsPin = 5;

WebServer server(80);  // Initialize the web server on port 80

void listFiles() {
  Serial.println("Listing files...");
  File root = SD.open("/");
  String htmlResponse = "<!DOCTYPE html><html><head><title>File Browser</title>"
                        "<style>body { font-family: Arial, sans-serif; background-color: #f4f4f9; }"
                        ".container { max-width: 600px; margin: auto; background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }"
                        "h2 { color: #333; }"
                        "ul { list-style-type: none; padding: 0; }"
                        "li { margin: 8px 0; }"
                        "a { text-decoration: none; color: #007bff; }"
                        "</style></head><body><div class='container'><h2>SD Card File Browser</h2>"
                        "<nav><a href='/'>Home</a> | <a href='/newfile'>Create New File</a></nav><br>"
                        "<ul>";

  while (File file = root.openNextFile()) {
    String fileName = file.name();
    htmlResponse +=  "<li><a href='/viewfile?name=" + String("/") + fileName + "'>" + fileName + "</a></li>";
    file.close();
  }

  htmlResponse += "</ul></div></body></html>";

  server.send(200, "text/html", htmlResponse);
}

void handleNewFilePage() {
  String htmlResponse = "<!DOCTYPE html><html><head><title>Create New File</title>"
                        "<style>body { font-family: Arial, sans-serif; background-color: #f4f4f9; }"
                        ".container { max-width: 600px; margin: auto; background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }"
                        "h2 { color: #333; }"
                        "</style></head><body><div class='container'><h2>Create New File</h2>"
                        "<form action='/newfile' method='POST'>"
                        "<input type='text' name='filename' placeholder='File name' required><br><br>"
                        "<textarea name='fileContent' rows='4' placeholder='File content'></textarea><br><br>"
                        "<input type='submit' value='Create File'>"
                        "</form><br><a href='/'>Back to file list</a></div></body></html>";

  server.send(200, "text/html", htmlResponse);
}

void handleViewFile() {
  if (server.hasArg("name")) {
    String fileName = server.arg("name");
    Serial.println("Viewing file: " + fileName);

    if (SD.exists(fileName)) {
      File file = SD.open(fileName);
      String fileContent = "";

      while (file.available()) {
        fileContent += (char)file.read();
      }
      file.close();

      String htmlResponse = "<!DOCTYPE html><html><head><title>File Viewer</title>"
                            "<style>body { font-family: Arial, sans-serif; background-color: #f4f4f9; }"
                            ".container { max-width: 600px; margin: auto; background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }"
                            "h2 { color: #333; }"
                            ".content { white-space: pre-wrap; background: #f9f9f9; padding: 10px; border-radius: 5px; border: 1px solid #ddd; }"
                            "</style></head><body><div class='container'><h2>Contents of " + fileName + "</h2><div class='content'>" + fileContent + "</div>"
                            "<form action='/editfile' method='POST'><input type='hidden' name='filename' value='" + fileName + "'>"
                            "<textarea name='fileContent' rows='4'>" + fileContent + "</textarea><br><br>"
                            "<input type='submit' value='Save Changes'></form><br><a href='/'>Back to file list</a></div></body></html>";

      server.send(200, "text/html", htmlResponse);
    } else {
      Serial.println("File not found: " + fileName);
      server.send(404, "text/html", "<html><body><h2>File not found</h2><a href='/'>Back to file list</a></body></html>");
    }
  } else {
    Serial.println("Bad request: 'name' parameter missing.");
    server.send(400, "text/html", "<html><body><h2>Bad request: 'name' parameter missing</h2></body></html>");
  }
}

void handleEditFile() {
  if (server.hasArg("filename") && server.hasArg("fileContent")) {
    String fileName = server.arg("filename");
    String newContent = server.arg("fileContent");
    Serial.println("Editing file: " + fileName);

    if (SD.exists(fileName)) {
      SD.remove(fileName);

      File file = SD.open(fileName, FILE_WRITE);
      if (file) {
        file.print(newContent);
        file.close();
        Serial.println("File updated successfully: " + fileName);
        server.send(200, "text/html", "<html><body><h2>File updated successfully</h2><a href='/'>Back to file list</a></body></html>");
      } else {
        Serial.println("Failed to open file for writing: " + fileName);
        server.send(500, "text/html", "<html><body><h2>Failed to open file for writing</h2><a href='/'>Back to file list</a></body></html>");
      }
    } else {
      Serial.println("File not found for editing: " + fileName);
      server.send(404, "text/html", "<html><body><h2>File not found</h2><a href='/'>Back to file list</a></body></html>");
    }
  } else {
    Serial.println("Bad request: Missing 'filename' or 'fileContent' parameter.");
    server.send(400, "text/html", "<html><body><h2>Bad request: Missing 'filename' or 'fileContent' parameter</h2></body></html>");
  }
}

void handleNewFile() {
  if (server.hasArg("filename") && server.hasArg("fileContent")) {
    String fileName = server.arg("filename");
    String fileContent = server.arg("fileContent");
    Serial.println("Creating new file: " + fileName);

    if (SD.exists(fileName)) {
      Serial.println("File already exists: " + fileName);
      server.send(400, "text/html", "<html><body><h2>File already exists</h2><a href='/'>Back to file list</a></body></html>");
    } else {
      File file = SD.open(fileName, FILE_WRITE);
      if (file) {
        file.print(fileContent);
        file.close();
        Serial.println("File created successfully: " + fileName);
        server.send(200, "text/html", "<html><body><h2>File created successfully</h2><a href='/'>Back to file list</a></body></html>");
      } else {
        Serial.println("Failed to create file: " + fileName);
        server.send(500, "text/html", "<html><body><h2>Failed to create file</h2><a href='/'>Back to file list</a></body></html>");
      }
    }
  } else {
    Serial.println("Bad request: Missing 'filename' or 'fileContent' parameter.");
    server.send(400, "text/html", "<html><body><h2>Bad request: Missing 'filename' or 'fileContent' parameter</h2></body></html>");
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println("IP Address:");
  Serial.println(WiFi.localIP());

  // Initialize the SD card
  if (!SD.begin(sdCardCsPin)) {
    Serial.println("Failed to initialize SD card");
    return;
  }
  Serial.println("SD card initialized.");

  // Define routes
  server.on("/", listFiles);
  server.on("/viewfile", handleViewFile);
  server.on("/editfile", HTTP_POST, handleEditFile);
  server.on("/newfile", HTTP_POST, handleNewFile);
  server.on("/newfile", handleNewFilePage); // Add new file page

  // Start the web server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();  // Handle incoming client requests
}
