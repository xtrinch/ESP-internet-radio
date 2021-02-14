#include "stream-mp3.h"

// A connection is made to a shoutcast server.  The server starts with some
// info in the header in readable ascii, ending with a double CRLF, like:
//  icy-name:Classic Rock Florida - SHE Radio
//  icy-genre:Classic Rock 60s 70s 80s Oldies Miami South Florida
//  icy-url:http://www.ClassicRockFLorida.com
//  content-type:audio/mpeg
//  icy-pub:1
//  icy-metaint:32768          - Metadata after 32768 bytes of MP3-data
//  icy-br:128                 - in kb/sec (for Ogg this is like "icy-br=Quality 2")
//
// After a double CRLF is received, the server starts sending mp3- or Ogg-data.  For mp3, this
// data may contain metadata (non mp3) after every "metaint" mp3 bytes.
// The metadata is empty in most cases, but if any is available the content will be
// presented on the TFT.

bool              chunked = false;                     // Station provides chunked transfer
int               chunkcount = 0;                      // Counter for chunked transfer
int               bitrate;                             // Bitrate in kb/sec
int               mbitrate;                            // Measured bitrate
int               metaint = 0;                         // Number of databytes between metadata
char              metalinebf[METASIZ + 1];             // Buffer for metaline/ID3 tags
int16_t           metalinebfx;                         // Index for metalinebf
String            host;                                // The URL to connect to or file to play
int               metacount;                           // Number of bytes in metadata
WiFiClient        mp3client;                           // An instance of the mp3 client, also used for OTA
String            playlist;                            // The URL of the specified playlist
uint32_t          totalcount = 0;                      // Counter mp3 data
int               datacount;                           // Counter databytes before metadata
uint32_t          clength;                             // Content length found in http header
qdata_struct      outchunk;                            // Data to queue
qdata_struct      inchunk;                             // Data from queue
uint8_t*          outqp = outchunk.buf;                // Pointer to buffer in outchunk
uint32_t          max_mp3loop_time = 0;                // To check max handling time in mp3loop (msec)
QueueHandle_t     dataqueue;                           // Queue for mp3 datastream
bool              hostreq = false;                     // Request for new host
uint8_t           tmpbuff[6000];                       // Input buffer for mp3 or data stream 

// forward declarations
void        showstreamtitle(const char* ml);
void        stop_mp3client ();

// Parses line with XML data and put result in variable specified by parameter.      
void xmlparse(String &line, const char *selstr, String &res) {
  String sel = "</";                                 // Will be like "</status-code"
  int    inx;                                        // Position of "</..." in line

  sel += selstr;                                     // Form searchstring
  if (line.endsWith(sel)) {                        // Is this the line we are looking for?
    inx = line.indexOf(sel);                       // Get position of end tag
    res = line.substring(0, inx);                  // Set result
  }
}

// Parses streams from XML data.                                    
// Example URL: http://playerservices.streamtheworld.com/api/livestream?version=1.5&mount=IHR_TRANAAC&lang=en
String xmlgethost (String mount) {
  const char* xmlhost = "playerservices.streamtheworld.com"; // XML data source
  const char* xmlget =  "GET /api/livestream"                  // XML get parameters
                        "?version=1.5"                         // API Version of IHeartRadio
                        "&mount=%sAAC"                         // MountPoint with Station Callsign
                        "&lang=en";                          // Language

  String   stationServer = "";                    // Radio stream server
  String   stationPort = "";                      // Radio stream port
  String   stationMount = "";                     // Radio stream Callsign
  uint16_t timeout = 0;                           // To detect time-out
  String   sreply = "";                           // Reply from playerservices.streamtheworld.com
  String   statuscode = "200";                    // Assume good reply
  char     tmpstr[200];                           // Full GET command, later stream URL
  String   urlout;                                // Result URL

  stop_mp3client(); // Stop any current wificlient connections.
  ardprintf("Connect to new iHeartRadio host: %s", mount.c_str());
  setdatamode(INIT);                            // Start default in metamode
  chunked = false;                                  // Assume not chunked
  sprintf(tmpstr, xmlget, mount.c_str());         // Create a GET commmand for the request
  ardprintf("%s", tmpstr);
  if (mp3client.connect(xmlhost, 80, 5000)) {           // Connect to XML stream
    ardprintf("Connected to %s", xmlhost);
    mp3client.print(String(tmpstr)+ " HTTP/1.1\r\n"
                      "Host: " + xmlhost + "\r\n"
                      "User-Agent: Mozilla/5.0\r\n"
                      "Connection: close\r\n\r\n");
    while(mp3client.available() == 0) {
      delay(200);                                 // Give server some time
      if (++timeout > 25) {                         // No answer in 5 seconds?
        ardprintf("Client Timeout !");
      }
    }
    ardprintf("XML parser processing...");
    while(mp3client.available()) {
      sreply = mp3client.readStringUntil('>');
      sreply.trim();
      // Search for relevant info in in reply and store in variable
      xmlparse(sreply, "status-code", statuscode);
      xmlparse(sreply, "ip",          stationServer);
      xmlparse(sreply, "port",        stationPort);
      xmlparse(sreply, "mount",       stationMount);
      if (statuscode != "200") {                    // Good result sofar?
        ardprintf("Bad xml status-code %s",         // No, show and stop interpreting
                   statuscode.c_str());
        tmpstr[0] = '\0';                          // Clear result
        break;
      }
    }
    // Check if all station values are stored
    if ((stationServer != "") && (stationPort != "") && (stationMount != "")) {
      sprintf(tmpstr, "%s:%s/%s_SC",                // Build URL for ESP-Radio to stream.
                stationServer.c_str(),
                stationPort.c_str(),
                stationMount.c_str());
      ardprintf("Found: %s", tmpstr);
    }
  } else {
    ardprintf("Can't connect to XML host!");       // Connection failed
    tmpstr[0] = '\0' ;
  }
  mp3client.stop();
  return String(tmpstr);                          // Return final streaming URL.
}

// Check if a line in the header is a reasonable headerline.        
// Normally it should contain something like "icy-xxxx:abcdef".     
bool chkhdrline(const char* str) {
  char    b;                                        // Byte examined
  int     len = 0;                                  // Lengte van de string

  while(( b = *str++)) {                           // Search to end of string
    len++;                                          // Update string length
    if (! isalpha(b)) {                           // Alpha (a-z, A-Z)
      if (b != '-') {                               // Minus sign is allowed
        if (b == ':') {                             // Found a colon?
          return(( len > 5)&&(len < 50));    // Yes, okay if length is okay
        } else {
          return false;                             // Not a legal character
        }
      }
    }
  }
  return false;                                     // End of string without colon
}

// If the line contains content-length information: set clength (content length counter).       
void scan_content_length(const char* metalinebf) {
  if (strstr(metalinebf, "Content-Length")) {       // Line contains content length
    clength = atoi(metalinebf + 15);                // Yes, set clength
    ardprintf("Content-Length is %d", clength);      // Show for debugging purposes
  }
}

// Connect to the Internet radio server specified by newpreset.     
bool connectToStation() {
  int         inx;                                // Position of ":" in hostname
  uint16_t    port = 80;                          // Port number for host
  String      extension = "/";                    // May be like "/mp3" in "skonto.ls.lv:8002/mp3"
  String      hostwoext = host;                   // Host without extension and portnumber
  String      auth;                               // For basic authentication

  stop_mp3client();                                // Disconnect if still connected
  ardprintf("Connect to new host %s", host.c_str());
  tftset(1, "");                                // Clear song and artist
  tftset(2, "Connecting...");                                // Clear song and artist
  setdatamode(INIT);                            // Start default in metamode
  chunked = false;                                // Assume not chunked
  // In the URL there may be an extension, like noisefm.ru:8000/play.m3u&t=.m3u
  inx = host.indexOf("/");                      // Search for begin of extension
  if (inx > 0) {                                  // Is there an extension?
    extension = host.substring(inx);            // Yes, change the default
    hostwoext = host.substring(0, inx);         // Host without extension
  }
  // In the host there may be a portnumber
  inx = hostwoext.indexOf(":");                 // Search for separator
  if (inx >= 0)                                  // Portnumber available?
  {
    port = host.substring(inx + 1 ).toInt();     // Get portnumber as integer
    hostwoext = host.substring(0, inx);         // Host without portnumber
  }
  ardprintf("Connect to %s on port %d, extension %s",
             hostwoext.c_str(), port, extension.c_str());

  ardprintf("Wifi stats: %d", WiFi.status());
  if (mp3client.connect(hostwoext.c_str(), port, 5000)) {
    ardprintf("Connected to server");
    mp3client.print(String("GET ")+
                      extension +
                      String(" HTTP/1.1\r\n")+
                      String("Host: ")+
                      hostwoext +
                      String("\r\n")+
                      String("Icy-MetaData:1\r\n")+
                      auth +
                      String("Connection: close\r\n\r\n"));
    return true ;
  }

  ardprintf("Request %s failed!", host.c_str());
  changeState("stop");
  tftset(2, "No connection");                                // Clear song and artist

  return false ;
}

// Handle the next byte of data from server.                        
// Chunked transfer encoding aware. Chunk extensions are not supported.              
void handlebyte_ch(uint8_t b ) {
  static int       chunksize = 0;                     // Chunkcount read from stream
  static int       LFcount;                           // Detection of end of header
  static bool      ctseen = false;                    // First line of header seen or not

  if (chunked && (datamode & (DATA | METADATA))) {
    if (chunkcount == 0)  {                           // Expecting a new chunkcount?
      if (b == '\r') {                               // Skip CR
        return ;
      } else if (b == '\n') {                          // LF ?
        chunkcount = chunksize;                      // Yes, set new count
        chunksize = 0;                               // For next decode
        return ;
      }
      // We have received a hexadecimal character.  Decode it and add to the result.
      b = toupper(b)- '0';                       // Be sure we have uppercase
      if (b > 9) {
        b = b - 7;                                   // Translate A..F to 10..15
      }
      chunksize =(chunksize << 4)+ b ;
      return  ;
    }
    chunkcount--;                                    // Update count to next chunksize block
  }
  if (datamode == DATA) {                            // Handle next byte of MP3/Ogg data
    *outqp++ = b ;
    if (outqp ==(outchunk.buf + sizeof(outchunk.buf))) { // Buffer full?
      // Send data to playtask queue.  If the buffer cannot be placed within 200 ticks,
      // the queue is full, while the sender tries to send more.  The chunk will be dis-
      // carded it that case.
      xQueueSend(dataqueue, &outchunk, 200);       // Send to queue
      outqp = outchunk.buf;                          // Item empty now
    }
    if (metaint) {                                   // No METADATA on Ogg streams or mp3 files
      if (--datacount == 0) {                        // End of datablock?
        setdatamode(METADATA);
        metalinebfx = -1;                            // Expecting first metabyte (counter)
      }
    }
    return ;
  }
  if (datamode == INIT) {                            // Initialize for header receive
    ctseen = false;                                  // Contents type not seen yet
    metaint = 0;                                     // No metaint found
    LFcount = 0;                                     // For detection end of header
    bitrate = 0;                                     // Bitrate still unknown
    ardprintf("Switch to HEADER");
    setdatamode(HEADER);                           // Handle header
    totalcount = 0;                                  // Reset totalcount
    metalinebfx = 0;                                 // No metadata yet
    metalinebf[0] = '\0' ;
  }
  if (datamode == HEADER) {                          // Handle next byte of MP3 header
    if (( b > 0x7F)||                               // Ignore unprintable characters
        (b == '\r')||                              // Ignore CR
        (b == '\0')) {                              // Ignore NULL
      // Yes, ignore
    }
    else if (b == '\n') {                            // Linefeed ?
      LFcount++;                                     // Count linefeeds
      metalinebf[metalinebfx] = '\0';                // Take care of delimiter
      if (chkhdrline(metalinebf)) {                // Reasonable input?
        ardprintf("Headerline: %s",                   // Show headerline
                   metalinebf);
        String metaline = String(metalinebf);      // Convert to string
        String lcml = metaline;                      // Use lower case for compare
        lcml.toLowerCase();
        if (lcml.startsWith("location: http://")) { // Redirection?
          host = metaline.substring(17);           // Yes, get new URL
          hostreq = true;                            // And request this one
        }
        if (lcml.indexOf("content-type")>= 0) {    // Line with "Content-Type: xxxx/yyy"
          ctseen = true;                             // Yes, remember seeing this
          String ct = metaline.substring(13);      // Set contentstype. Not used yet
          ct.trim();
          ardprintf("%s seen.", ct.c_str());
        }
        if (lcml.startsWith("icy-br:")) {
          bitrate = metaline.substring(7).toInt();    // Found bitrate tag, read the bitrate
          if (bitrate == 0)                         // For Ogg br is like "Quality 2"
          {
            bitrate = 87;                            // Dummy bitrate
          }
        }
        else if (lcml.startsWith ("icy-metaint:")) {
          metaint = metaline.substring(12).toInt();   // Found metaint tag, read the value
        }
        else if (lcml.startsWith("icy-name:")) {
          String icyname = metaline.substring(9);            // Get station name
          icyname.trim();                             // Remove leading and trailing spaces
          tftset(2, icyname.c_str());                      // Set screen segment bottom part
        }
        else if (lcml.startsWith("transfer-encoding:")) {
          // Station provides chunked transfer
          if (lcml.endsWith("chunked"))
          {
            chunked = true;                          // Remember chunked transfer mode
            chunkcount = 0;                          // Expect chunkcount in DATA
          }
        }
      }
      metalinebfx = 0;                               // Reset this line
      if (( LFcount == 2)&& ctseen) {              // Content type seen and a double LF?
        ardprintf("Switch to DATA, bitrate is %d"     // Show bitrate
                   ", metaint is %d",                  // and metaint
                   bitrate, metaint);
        setdatamode(DATA);                         // Expecting data now
        datacount = metaint;                         // Number of bytes before first metadata
        queuefunc(QSTARTSONG);                     // Queue a request to start song
      }
    } else {
      metalinebf[metalinebfx++] = (char)b;           // Normal character, put new char in metaline
      if (metalinebfx >= METASIZ)                   // Prevent overflow
      {
        metalinebfx-- ;
      }
      LFcount = 0;                                   // Reset double CRLF detection
    }
    return ;
  }
  if (datamode == METADATA) {                        // Handle next byte of metadata
    if (metalinebfx < 0) {                           // First byte of metadata?
      metalinebfx = 0;                               // Prepare to store first character
      metacount = b * 16 + 1;                        // New count for metadata including length byte
      if (metacount > 1 ) {
        ardprintf("Metadata block %d bytes",
                   metacount - 1);                   // Most of the time there are zero bytes of metadata
      }
    } else {
      metalinebf[metalinebfx++] = (char)b;           // Normal character, put new char in metaline
      if (metalinebfx >= METASIZ) {                  // Prevent overflow
        metalinebfx-- ;
      }
    }
    if (--metacount == 0 ) {
      metalinebf[metalinebfx] = '\0';                // Make sure line is limited
      if (strlen(metalinebf)) {                    // Any info present?
        // metaline contains artist and song name.  For example:
        // "StreamTitle='Don McLean - American Pie';StreamUrl='';"
        // Sometimes it is just other info like:
        // "StreamTitle='60s 03 05 Magic60s';StreamUrl='';"
        // Isolate the StreamTitle, remove leading and trailing quotes if present.
        showstreamtitle(metalinebf);               // Show artist and title if present in metadata
      }
      if (metalinebfx  >(METASIZ - 10)) {          // Unlikely metaline length?
        ardprintf("Metadata block too long! Skipping all Metadata from now on.");
        metaint = 0;                                 // Probably no metadata
      }
      datacount = metaint;                           // Reset data count
      //bufcnt = 0;                                  // Reset buffer count
      setdatamode(DATA);                           // Expecting data
    }
  }
}


// Show artist and songtitle if present in metadata.                
// Show always if full=true.                                        
void showstreamtitle(const char *ml) {
  char*             p1 ;
  char*             p2 ;
  char              streamtitle[150];          // Streamtitle from metadata

  if (strstr(ml, "StreamTitle=")) {
    ardprintf("Streamtitle found, %d bytes", strlen(ml));
    ardprintf(ml);
    p1 = (char*)ml + 12;                      // Begin of artist and title
    if (( p2 = strstr(ml, ";")))         // Search for end of title
    {
      if (*p1 == '\'')                       // Surrounded by quotes?
      {
        p1++ ;
        p2-- ;
      }
      *p2 = '\0';                             // Strip the rest of the line
    }
    // Save last part of string as streamtitle.  Protect against buffer overflow
    strncpy(streamtitle, p1, sizeof(streamtitle));
    streamtitle[sizeof(streamtitle)- 1] = '\0' ;
  } else {
    // Info probably from playlist
    strncpy(streamtitle, ml, sizeof(streamtitle));
    streamtitle[sizeof(streamtitle)- 1] = '\0' ;
  }
  // Save for status request from browser and for MQTT
  if (( p1 = strstr(streamtitle, " - "))) { // look for artist/title separator
    p2 = p1 + 3;                              // 2nd part of text at this position
    *p1++ = '\n';                           // Found: replace 3 characters by newline
    if (*p2 == ' ')                          // Leading space in title?
    {
      p2++ ;
    }
    strcpy(p1, p2);                         // Shift 2nd part of title 2 or 3 places
  }
  tftset(1, streamtitle);                   // Set screen segment text middle part
}

// Disconnect from the server.                                      
void stop_mp3client () {
  ardprintf ("FLUSH: Stopping client");               // Stop connection to host
  while(mp3client.connected()) {
    // client.flush() removes only the current packet characters from the socket. By the time you call client.stop(), 
    // the socket may already have characters from the next packet in it.
    while(mp3client.available()) mp3client.flush();
    mp3client.stop();
    delay(500);
  }
  mp3client.flush();                              // Flush stream client
  mp3client.stop();                               // Stop stream client
}

// Called from the mail loop() for the mp3 functions.               
// A connection to an MP3 server is active and we are ready to receive data.         
// Normally there is about 2 to 4 kB available in the data stream.  This depends on the sender. 
void mp3loop() {
  uint32_t        maxchunk;                            // Max number of bytes to read
  int             res = 0;                             // Result reading from mp3 stream
  uint32_t        av = 0;                              // Available in stream
  String          nodeID;                              // Next nodeID of track on SD
  uint32_t        timing;                              // Startime and duration this function
  uint32_t        qspace;                              // Free space in data queue

  // Try to keep the Queue to playtask filled up by adding as much bytes as possible
  if (isPlaying()) {
    timing = millis();                                  // Start time this function
    maxchunk = sizeof(tmpbuff);                         // Reduce byte count for this mp3loop()
    qspace = uxQueueSpacesAvailable( dataqueue)*       // Compute free space in data queue
             sizeof(qdata_struct);
    
    av = mp3client.available();                       // Available from stream
    if (av < maxchunk) {                             // Limit read size
      maxchunk = av;
    }
    if (maxchunk > qspace) {                         // Enough space in queue?
      maxchunk = qspace;                             // No, limit to free queue space
    }
    if (maxchunk > 1000) {                           // Only read if worthwile
      res = mp3client.read(tmpbuff, maxchunk);     // Read a number of bytes from the stream
    }
    for(int i = 0;i < res; i++) {
      handlebyte_ch(tmpbuff[i]);                     // Handle one byte
    }
    timing = millis() - timing;                        // Duration this function
    if (timing > max_mp3loop_time)                    // New maximum found?
    {
      max_mp3loop_time = timing;                       // Yes, set new maximum
      ardprintf("Duration mp3loop %d", timing);       // and report it
    }
  }
  if (datamode == STOPREQD) {                          // STOP requested?
    ardprintf("STOP requested");
    stop_mp3client();                                 // Disconnect if still connected
    chunked = false;                                   // Not longer chunked
    datacount = 0;                                     // Reset datacount
    outqp = outchunk.buf;                              // and pointer
    queuefunc(QSTOPSONG);                            // Queue a request to stop the song
    metaint = 0;                                       // No metaint known now
    setdatamode(STOPPED);                            // Yes, state becomes STOPPED
    return ;
  }
  if (newpreset != currentpreset) {          // New station or next from playlist requested?
    if (datamode != STOPPED) {                         // Yes, still busy?
      setdatamode(STOPREQD);                         // Yes, request STOP
    } else {
      host = String(hostFromConfig(newpreset));    // Lookup preset in preferences
      ardprintf("New preset/file requested (%d) from %s",
                 newpreset, host.c_str());
      if (host != "") {                                // Preset in ini-file?
        hostreq = true;                                 // Force this station as new preset
      } else {
        // This preset is not available, return to preset 0, will be handled in next mp3loop()
        ardprintf("No host for this preset");
        newpreset = 0;                        // Wrap to first station
      }
    }
  }
  if (hostreq) {                                        // New preset or station?
    hostreq = false ;
    currentpreset = newpreset;                         // Remember current preset
    connectToStation();                                   // Switch to new host
  }
}