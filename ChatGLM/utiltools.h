#ifndef UTIL_TOOL_H
#define UTIL_TOOL_H

String addHistoryToFile(DynamicJsonDocument &HistoryDoc, String role, String content) {
  String jsonStr;

  HistoryDoc["role"] = role;
  HistoryDoc["content"] = content;

  serializeJson(HistoryDoc, jsonStr);

  File file = SPIFFS.open(HistoryfilePath, "a");
  if (!file) {
    Serial.println(F("Failed to open history file for writing"));
    return "";
  }

  if (file.println(jsonStr)) {
    Serial.println(F("Data appended to file"));
  } else {
    Serial.println(F("Append failed"));
  }
  file.close();

  return jsonStr;
}

String loadHistoryFromFile() {
  String historyData = "";

  File file = SPIFFS.open(HistoryfilePath, "r");
  if (!file) {
    Serial.println(F("Failed to open history file for reading"));
    return historyData;
  }

  while (file.available()) {
    historyData += (char)file.read();
  }

  file.close();

  return historyData;
}

String setInputMessage() {
  String message = loadHistoryFromFile();
  if (!message.isEmpty()) {
    return message;
  } else {
    return String();
  }
}

/*
Since there are no regular expressions, the simplest way to handle our data is used here. 
The function replaces all patterns in the text, replaces them with replacements, and returns the result of the replacements.
*/

String replacePattern(String &text, String pattern, String &replacement) {
  String result = text;
  int index = result.indexOf(pattern);  // Find the location of the pattern in the text
  while (index != -1) {
    result = result.substring(0, index) + replacement + result.substring(index + pattern.length());  // carry out the replacement
    index = result.indexOf(pattern, index + replacement.length());                                   // Update Index
  }
  return result;
}


String lastMessages(String role, String messages) {
  String inputMessage = setInputMessage();
  DynamicJsonDocument input(327680);
  deserializeJson(input, inputMessage);

  input["role"] = role;
  input["content"] = messages;

  String texts;
  serializeJson(input, texts);

  String pattern = ",}";
  String replacement = "";
  String result = replacePattern(inputMessage + texts, pattern, replacement);
  return result;
}

String replaceDoubleBackslashes(String input) {
  input.replace("\\\\", "\\");
  input.replace("\\r\\n", ",");
  input.replace("\\n\\n", "\n");
  return input;
}


String messageJSON(String user_input, bool isStream) {
  DynamicJsonDocument messages(204800);
  DynamicJsonDocument json_request_body(327680);

  JsonObject systemMessage = messages.createNestedObject();
  systemMessage["role"] = system_role;
  systemMessage["content"] = system_content;

  JsonObject userMessage = messages.createNestedObject();
  userMessage["role"] = user_role;
  userMessage["content"] = lastMessages(user_role, user_input);

  json_request_body["model"] = "glm-4";  //Default is glm-4, You can change here for glm-3-turbo.
  //json_request_body["model"] = "glm-3-turbo";
  json_request_body["messages"] = messages;
  json_request_body["stream"] = isStream;
  json_request_body["do_sample"] = true;
  json_request_body["temperature"] = 0.6;
  json_request_body["top_p"] = 0.7;

  String json_string;
  serializeJson(json_request_body, json_string);

  String result = replaceDoubleBackslashes(json_string);

  Serial.println(result);

  return result;
}

bool FileSizeChecker() {
  File file = SPIFFS.open(HistoryfilePath, "r");
  if (!file) {
    Serial.println(F("Failed to open file!"));
    return false;  // Returns false if the file failed to open.
  }

  size_t fileSize = file.size();

  Serial.print(F("File Size is: "));
  Serial.println(fileSize);

  file.close();

  if (fileSize >= 5500) {
    if (SPIFFS.remove(HistoryfilePath)) {
      Serial.println(F("File deleted successfully!"));
      return true;  // Returns true if the file was deleted successfully
    } else {
      Serial.println(F("Failed to delete file!"));
      return false;  // Returns false if file deletion fails
    }
  } else {
    Serial.println(F("File size is within limit"));
    return false;  // Returns false if the file size is within the limit
  }
}


#endif