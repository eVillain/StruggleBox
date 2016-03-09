#include <stdlib.h>
#include <string.h>

#include "StatTracker.h"
#include "HyperVisor.h"
#include "TextManager.h"
#include "GFXHelpers.h"
#include "Renderer.h"

#include "Options.h"

StatTracker::StatTracker(Locator& locator) :
_locator(locator)
{
    // Whether StatTracker is visible
    s_showStats = false;
    
    lastBarNum = 0;
    
    // Renderer statistics
    s_rFPS = 0;
    s_rFrameDelta = 0;
    s_rTime = 0;
    s_pTime = 0;
    s_pCollisions = 0;
    s_pManifolds = 0;
    s_eTime = 0;
    s_eNum = 0;
    
    s_rNumTris = 0;
    s_rNumSegs = 0;
    s_rNumSpheres = 0;
    s_rNumLights2D = 0;
    s_rNumLights3D = 0;
    
    s_tNumJobs = 0;
    // Physics statistics
    //    int s_pBodies = 0;
    //    int s_pStaticBodies = 0;
    //    int s_pShapes = 0;
    //    int s_pStaticShapes = 0;
    //    int s_pArbiters = 0;
    //    int s_pPoints = 0;
    //    int s_pConstraints = 0;
    //    double s_pKE = 0.0;
}
StatTracker::~StatTracker() {
    
}
// Statistics setters
void StatTracker::SetRFPS( double newFPS ) { s_rFPS = newFPS; };
void StatTracker::SetRFrameDelta( double newFrameDelta ) { s_rFrameDelta = newFrameDelta; };
void StatTracker::SetRTime( double newTime ) { s_rTime = newTime; };
void StatTracker::SetPTime( double newTime ) { s_pTime = newTime; };
void StatTracker::SetPCollisions( int newCollisions ) { s_pCollisions = newCollisions; };
void StatTracker::SetPManifodlds( const int newManifolds ) { s_pManifolds = newManifolds; };
void StatTracker::SetETime( double newTime ) { s_eTime = newTime; };
void StatTracker::SetENum( int newEntities ) { s_eNum = newEntities; };
void StatTracker::SetRNumTris( int newNumTris ) { s_rNumTris = newNumTris; };
void StatTracker::SetRNumSegs( int newNumSegs ) { s_rNumSegs = newNumSegs; };
void StatTracker::SetRNumSpheres( int newNumSpheres ) { s_rNumSpheres = newNumSpheres; };
void StatTracker::SetRNumLights2D( int newNumLights2D ) { s_rNumLights2D = newNumLights2D; };
void StatTracker::SetRNumLights3D( int newNumLights3D ) { s_rNumLights3D = newNumLights3D; };
void StatTracker::SetTNumJobs( const int newJobs ) { s_tNumJobs = newJobs; };
// Physics
//    void SetPBodies( int newBodies ) { s_pBodies = newBodies; };
//    void SetPStaticBodies( int newStaticBodies ) { s_pStaticBodies = newStaticBodies; };
//    void SetPShapes( int newShapes ) { s_pShapes = newShapes; };
//    void SetPStaticShapes( int newStaticShapes ) { s_pStaticShapes = newStaticShapes; };
//    void SetPArbiters( int newArbiters ) { s_pArbiters = newArbiters; };
//    void SetPPoints( int newPoints ) { s_pPoints = newPoints; };
//    void SetPConstraints( int newConstraints ) { s_pConstraints = newConstraints; };
//    void SetPKE( double newKE ) { s_pKE = newKE; };

void StatTracker::ToggleStats()
{
    s_showStats = !s_showStats;
    if (s_showStats) { DisplayStats(); }
    else { HideStats(); }
}

void StatTracker::DisplayStats()
{
    int x = _locator.Get<Options>()->GetOptionDataPtr<int>("r_resolutionX")/2-300;
    int y = _locator.Get<Options>()->GetOptionDataPtr<int>("r_resolutionY")/2-40;
    TextManager* textMan = _locator.Get<TextManager>();
    
    int fontSize = 20;
    int labelID = textMan->AddText( "Stats:", glm::vec3(x, y, 0.0), true, 32, FONT_MENU, 0.0f );
    labelVect.push_back( labelID );
    y -= 32;
    std::string fpsLabel = ("FPS: XX");
    labelID = textMan->AddText(fpsLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string frameDeltaLabel = ("Frame delta: XX");
    labelID = textMan->AddText(frameDeltaLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string renderTimeLabel = ("Render time: XX");
    labelID = textMan->AddText(renderTimeLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string physTimeLabel = ("Physics: XX/XX (XXms)");
    labelID = textMan->AddText(physTimeLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string entityTimeLabel = ("Entities: XX (XXms)");
    labelID = textMan->AddText(entityTimeLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string trisRenderedLabel = ("Triangles: XX");
    labelID = textMan->AddText(trisRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string spheresRenderedLabel = ("Spheres: XX");
    labelID = textMan->AddText(spheresRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string segsRenderedLabel = ("Lines: XX");
    labelID = textMan->AddText(segsRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize*2;
    std::string lights2DRenderedLabel = ("Lights 2D: XX");
    labelID = textMan->AddText(lights2DRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
    y -= fontSize;
    std::string lights3DRenderedLabel = ("Lights 3D: XX");
    labelID = textMan->AddText(lights3DRenderedLabel, glm::vec3(x, y, 0.0), true, fontSize, FONT_JURA, 0.0f);
    labelVect.push_back( labelID );
//    y -= fontSize*2;
}

void StatTracker::HideStats()
{
    TextManager* textMan = _locator.Get<TextManager>();
    for (unsigned int i=0; i < labelVect.size(); i++ ) {
        textMan->RemoveText( labelVect[i] );
    }
    labelVect.clear();
}

void StatTracker::UpdateStats( Renderer* renderer )
{
    // Update stat labels with new values
    if ( !s_showStats ) { return; }
    int labelIDPos = 1;
    TextManager* textMan = _locator.Get<TextManager>();

    std::string fpsLabel = ("FPS: ");
    fpsLabel.append(doubleToString(s_rFPS, 5));
    textMan->UpdateText(labelVect[labelIDPos], fpsLabel);
    labelIDPos++;
    std::string frameDeltaLabel = ("Frame delta: ");
    frameDeltaLabel.append(doubleToString(s_rFrameDelta, 5));
    frameDeltaLabel.append("ms");
    textMan->UpdateText(labelVect[labelIDPos], frameDeltaLabel);
    labelIDPos++;
    std::string renderTimeLabel = ("Render time: ");
    renderTimeLabel.append(doubleToString(s_rTime, 5));
    renderTimeLabel.append("ms");
    textMan->UpdateText(labelVect[labelIDPos], renderTimeLabel);
    labelIDPos++;
    std::string physTimeLabel = ("Physics: ");
    physTimeLabel.append(intToString(s_pCollisions));
    physTimeLabel.append(" / ");
    physTimeLabel.append(intToString(s_pManifolds));
    physTimeLabel.append(" (");
    physTimeLabel.append(doubleToString(s_pTime, 5));
    physTimeLabel.append("ms)");
    textMan->UpdateText(labelVect[labelIDPos], physTimeLabel);

    // Testing line and bar graph rendering
    barGraphInts[lastBarNum] = s_pCollisions;
    barGraphFloats[lastBarNum] = s_pTime;
    barGraphInts2[lastBarNum] = s_tNumJobs;
    s_pManifoldsArray[lastBarNum] = s_pManifolds;
    
    for (int i=0; i<lastBarNum; i++) {
        float pCollHeight = barGraphInts[i]*0.5f;;
        renderer->Draw2DRect(glm::vec2(200+i*2,200+pCollHeight), 2.0f, 2.0f*pCollHeight, COLOR_GREEN, COLOR_GREEN);
        if ( i > 0 ) {
            int i2 = i-1;
            float timeHeight = barGraphFloats[i];
            float timeHeight2 = barGraphFloats[i2];
            renderer->Buffer2DLine(glm::vec2(200+(i2*2),200+timeHeight2), glm::vec2(200+(i*2),200+timeHeight), COLOR_RED, COLOR_RED);
            float jobsHeight = barGraphInts2[i];
            float jobsHeight2 = barGraphInts2[i2];
            renderer->Buffer2DLine(glm::vec2(200+(i2*2),200+jobsHeight2), glm::vec2(200+(i*2),200+jobsHeight), COLOR_YELLOW, COLOR_YELLOW);
            float manifoldsHeight = s_pManifoldsArray[i];
            float manifoldsHeight2 = s_pManifoldsArray[i2];
            renderer->Buffer2DLine(glm::vec2(200+(i2*2),200+manifoldsHeight2), glm::vec2(200+(i*2),200+manifoldsHeight), COLOR_PURPLE, COLOR_PURPLE);
        }
        
    }
    renderer->Render2DLines();
    lastBarNum++;
    if ( lastBarNum >= 100 ) lastBarNum = 0;

    labelIDPos++;
    std::string entityTimeLabel = ("Entities: ");
    entityTimeLabel.append(intToString(s_eNum));
    entityTimeLabel.append(" (");

    entityTimeLabel.append(doubleToString(s_eTime, 5));
    entityTimeLabel.append("ms)");
    textMan->UpdateText(labelVect[labelIDPos], entityTimeLabel);
    labelIDPos++;
    
    std::string trisRenderedLabel = ("Triangles: ");
    trisRenderedLabel.append(intToString( s_rNumTris ));
    textMan->UpdateText(labelVect[labelIDPos], trisRenderedLabel);
    labelIDPos++;
    std::string segsRenderedLabel = ("Lines: ");
    segsRenderedLabel.append(intToString( s_rNumSegs ));
    textMan->UpdateText(labelVect[labelIDPos], segsRenderedLabel);
    labelIDPos++;
    std::string spheresRenderedLabel = ("Spheres: ");
    spheresRenderedLabel.append(intToString( s_rNumSpheres ));
    textMan->UpdateText(labelVect[labelIDPos], spheresRenderedLabel);
    labelIDPos++;
    std::string lights2DRenderedLabel = ("Lights 2D: ");
    lights2DRenderedLabel.append(intToString( s_rNumLights2D ));
    textMan->UpdateText(labelVect[labelIDPos], lights2DRenderedLabel);
    labelIDPos++;
    std::string lights3DRenderedLabel = ("Lights 3D: ");
    lights3DRenderedLabel.append(intToString( s_rNumLights3D ));
    textMan->UpdateText(labelVect[labelIDPos], lights3DRenderedLabel);
    labelIDPos++;
}


