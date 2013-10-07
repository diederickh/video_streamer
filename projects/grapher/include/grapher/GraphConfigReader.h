/*

---------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl  
                                          www.twitter.com/roxlu
              
---------------------------------------------------------------------------------

  # GraphConfig

  Used to load server and client graph configs.
  The server uses it for settings like window size, and tcp address.
  The client uses it to define the graphs you want to use.
  

  _Server config example, must be in the same path as the executable_

  ````xml
   <settings>
     <server>
       <address>tcp://127.0.0.1:7878</address>
       <width>600</width>
       <height>400</height>
     </server>
   </settings>
  ````

  _Client config example_

  ````xml
  <settings>
    <server>
      <address>tcp://127.0.0.1:7878</address> <!-- connect to the server on this address, server settings should use the same -->
    </server>
    <graphs>
      <graph>
        <id>0</id> <!-- must be an unique id, this is used to add data -->
        <width>700</width> <!-- width of the drawing area of the graph --> 
        <height>300</height> <!-- height of the drawing area of the graph -->
        <xtics>10</xtics> <!-- number of xtics on the horizontal axis -->
        <ytics>5</ytics> <!-- number of ytics on the vertical axis -->
        <x>10</x> <!-- x position, use to position the graph -->
        <y>15</y> <!-- y position, use to position the graph -->
        <min>-100</min>  <!-- min range to draw -->
        <max>100</max> <!-- max range to draw -->
      </graph>
      <graph>
        <id>1</id> <!-- must be an unique id, this is used to add data -->
        <width>700</width> <!-- width of the drawing area of the graph --> 
        <height>200</height> <!-- height of the drawing area of the graph -->
        <xtics>10</xtics> <!-- number of xtics on the horizontal axis -->
        <ytics>5</ytics> <!-- number of ytics on the vertical axis -->
        <x>10</x> <!-- x position, use to position the graph -->
        <y>15</y> <!-- y position, use to position the graph -->
        <min>-100</min>  <!-- min range to draw -->
        <max>100</max> <!-- max range to draw -->
      </graph>
  
    </graphs>
  </settings>
  ````

 */
#ifndef GRAPH_CONFIG_READER_H
#define GRAPH_CONFIG_READER_H

#include <grapher/Config.h>
#include <grapher/Types.h>
#include <string>

class GraphConfigReader {
 public:
  bool loadServerConfig(std::string filepath, ServerSettings& result);
  bool loadClientConfig(std::string filepath, ClientSettings& result);
};

#endif
