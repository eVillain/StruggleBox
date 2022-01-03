#include "StatTracker.h"

#include "GFXHelpers.h"
#include "Renderer.h"
#include "Options.h"
#include "Log.h"

#include <stdlib.h>
#include <string.h>

StatTracker::StatTracker(
	Options& options) :
	_options(options)
{
	Log::Info("[StatTracker] constructor, instance at %p", this);
}
StatTracker::~StatTracker()
{
	Log::Info("[StatTracker] destructor, instance at %p", this);
}

void StatTracker::trackFloatValue(const float value, const std::string& name)
{
    m_floatStats[name] = value;
}

void StatTracker::trackIntValue(const int32_t value, const std::string& name)
{
    m_intStats[name] = value;
}

void StatTracker::DisplayStats()
{
    int x = _options.getOption<int>("r_resolutionX")/2-300;
    int y = _options.getOption<int>("r_resolutionY")/2-40;
    
    //int fontSize = 20;
    //int labelID = _text->AddText( "Stats:", glm::vec3(x, y, 0.0), true, 32, FONT_MENU, 0.0f );
    //labelVect.push_back( labelID );
    //y -= 32;
    //std::string fpsLabel = ("FPS: XX");
    //labelID = _text->AddText(fpsLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string frameDeltaLabel = ("Frame delta: XX");
    //labelID = _text->AddText(frameDeltaLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string renderTimeLabel = ("Render time: XX");
    //labelID = _text->AddText(renderTimeLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string physTimeLabel = ("Physics: XX/XX (XXms)");
    //labelID = _text->AddText(physTimeLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string entityTimeLabel = ("Entities: XX (XXms)");
    //labelID = _text->AddText(entityTimeLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string trisRenderedLabel = ("Triangles: XX");
    //labelID = _text->AddText(trisRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string spheresRenderedLabel = ("Spheres: XX");
    //labelID = _text->AddText(spheresRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string segsRenderedLabel = ("Lines: XX");
    //labelID = _text->AddText(segsRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize*2;
    //std::string lights2DRenderedLabel = ("Lights 2D: XX");
    //labelID = _text->AddText(lights2DRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
    //y -= fontSize;
    //std::string lights3DRenderedLabel = ("Lights 3D: XX");
    //labelID = _text->AddText(lights3DRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    //labelVect.push_back( labelID );
//    y -= fontSize*2;
}

void StatTracker::HideStats()
{
  //  for (unsigned int i=0; i < labelVect.size(); i++ ) {
		//_text->RemoveText( labelVect[i] );
  //  }
  //  labelVect.clear();
}

void StatTracker::UpdateStats( Renderer* renderer )
{
 //   // Update stat labels with new values
 //   if ( !s_showStats ) { return; }
 //   int labelIDPos = 1;

 //   std::string fpsLabel = ("FPS: ");
 //   fpsLabel.append(doubleToString(s_rFPS, 5));
	//_text->UpdateText(labelVect[labelIDPos], fpsLabel);
 //   labelIDPos++;
 //   std::string frameDeltaLabel = ("Frame delta: ");
 //   frameDeltaLabel.append(doubleToString(s_rFrameDelta, 5));
 //   frameDeltaLabel.append("ms");
	//_text->UpdateText(labelVect[labelIDPos], frameDeltaLabel);
 //   labelIDPos++;
 //   std::string renderTimeLabel = ("Render time: ");
 //   renderTimeLabel.append(doubleToString(s_rTime, 5));
 //   renderTimeLabel.append("ms");
	//_text->UpdateText(labelVect[labelIDPos], renderTimeLabel);
 //   labelIDPos++;
 //   std::string physTimeLabel = ("Physics: ");
 //   physTimeLabel.append(intToString(s_pCollisions));
 //   physTimeLabel.append(" / ");
 //   physTimeLabel.append(intToString(s_pManifolds));
 //   physTimeLabel.append(" (");
 //   physTimeLabel.append(doubleToString(s_pTime, 5));
 //   physTimeLabel.append("ms)");
	//_text->UpdateText(labelVect[labelIDPos], physTimeLabel);

 //   // Testing line and bar graph rendering
 //   barGraphInts[lastBarNum] = s_pCollisions;
 //   barGraphFloats[lastBarNum] = s_pTime;
 //   barGraphInts2[lastBarNum] = s_tNumJobs;
 //   s_pManifoldsArray[lastBarNum] = s_pManifolds;
 //   
 //   for (int i=0; i<lastBarNum; i++) {
 //       float pCollHeight = barGraphInts[i]*0.5f;;
 //       renderer->Draw2DRect(glm::vec2(200+i*2,200+pCollHeight), 2.0f, 2.0f*pCollHeight, COLOR_GREEN, COLOR_GREEN);
 //       if ( i > 0 ) {
 //           int i2 = i-1;
 //           float timeHeight = barGraphFloats[i];
 //           float timeHeight2 = barGraphFloats[i2];
 //           renderer->Buffer2DLine(glm::vec2(200+(i2*2),200+timeHeight2), glm::vec2(200+(i*2),200+timeHeight), COLOR_RED, COLOR_RED);
 //           float jobsHeight = barGraphInts2[i];
 //           float jobsHeight2 = barGraphInts2[i2];
 //           renderer->Buffer2DLine(glm::vec2(200+(i2*2),200+jobsHeight2), glm::vec2(200+(i*2),200+jobsHeight), COLOR_YELLOW, COLOR_YELLOW);
 //           float manifoldsHeight = s_pManifoldsArray[i];
 //           float manifoldsHeight2 = s_pManifoldsArray[i2];
 //           renderer->Buffer2DLine(glm::vec2(200+(i2*2),200+manifoldsHeight2), glm::vec2(200+(i*2),200+manifoldsHeight), COLOR_PURPLE, COLOR_PURPLE);
 //       }
 //       
 //   }
 //   renderer->Render2DLines();
 //   lastBarNum++;
 //   if ( lastBarNum >= 100 ) lastBarNum = 0;

 //   labelIDPos++;
 //   std::string entityTimeLabel = ("Entities: ");
 //   entityTimeLabel.append(intToString(s_eNum));
 //   entityTimeLabel.append(" (");

 //   entityTimeLabel.append(doubleToString(s_eTime, 5));
 //   entityTimeLabel.append("ms)");
	//_text->UpdateText(labelVect[labelIDPos], entityTimeLabel);
 //   labelIDPos++;
 //   
 //   std::string trisRenderedLabel = ("Triangles: ");
 //   trisRenderedLabel.append(intToString( s_rNumTris ));
	//_text->UpdateText(labelVect[labelIDPos], trisRenderedLabel);
 //   labelIDPos++;
 //   std::string segsRenderedLabel = ("Lines: ");
 //   segsRenderedLabel.append(intToString( s_rNumSegs ));
	//_text->UpdateText(labelVect[labelIDPos], segsRenderedLabel);
 //   labelIDPos++;
 //   std::string spheresRenderedLabel = ("Spheres: ");
 //   spheresRenderedLabel.append(intToString( s_rNumSpheres ));
	//_text->UpdateText(labelVect[labelIDPos], spheresRenderedLabel);
 //   labelIDPos++;
 //   std::string lights2DRenderedLabel = ("Lights 2D: ");
 //   lights2DRenderedLabel.append(intToString( s_rNumLights2D ));
	//_text->UpdateText(labelVect[labelIDPos], lights2DRenderedLabel);
 //   labelIDPos++;
 //   std::string lights3DRenderedLabel = ("Lights 3D: ");
 //   lights3DRenderedLabel.append(intToString( s_rNumLights3D ));
	//_text->UpdateText(labelVect[labelIDPos], lights3DRenderedLabel);
 //   labelIDPos++;
}


