#include "Options.h"
#include "Dictionary.h"

#include "FileUtil.h"

#include "Console.h"
#include "CommandProcessor.h"

static bool s_debugOutput = false;

Options::Options()
{
    load();
}

void Options::load()
{
    bool loaded = false;
    std::string path = FileUtil::GetPath();
    if ( FileUtil::DoesFileExist(path, "Options.plist" ) ) {
        path.append("Options.plist");
        if ( load(path.c_str()) ) {
            loaded = true;
            if ( s_debugOutput ) Console::Print("Loaded options file");
        } else {
            if ( s_debugOutput ) Console::Print("Failed to load options file");
        }
    } else {
        if ( s_debugOutput ) Console::Print("No options file to load");
    }
    if (!loaded) {
        setDefaults();
        printDebugInfo();
        save();
    } else {
        addConsoleVars();
    }
}

void Options::save()
{
    std::string path = FileUtil::GetPath();
    path.append("Options.plist");
    if ( save(path.c_str()) ) {
        if ( s_debugOutput ) Console::Print("Saved options file");
    } else {
        if ( s_debugOutput ) Console::Print("Failed to save options file");
    }
}

/// Prints out options data values
void Options::printDebugInfo()
{
    std::map<const std::string, Attribute*>::iterator it;
    for (it = m_Attributes.begin(); it != m_Attributes.end(); it++) {
        printf("Options Key: %s, Data: %s\n",
               it->first.c_str(),
               it->second->GetValueString().c_str());
    }
}

void Options::addConsoleVars()
{
    std::map<const std::string, Attribute*>::iterator it;
    for ( it = m_Attributes.begin(); it != m_Attributes.end(); it++ ) {
        if ( it->second->IsType<bool>()) {
            Console::AddVar(it->second->as<bool>(), it->first);
        } else if ( it->second->IsType<int>()) {
            Console::AddVar(it->second->as<int>(), it->first);
        } else if ( it->second->IsType<float>()) {
            Console::AddVar(it->second->as<float>(), it->first);
        } else if ( it->second->IsType<std::string>()) {
            Console::AddVar(it->second->as<std::string>(), it->first);
        }
    }
}

bool Options::save(const char *fileName)
{
    Dictionary dict;
    std::map<const std::string, Attribute*>::iterator it;
    for ( it = m_Attributes.begin(); it != m_Attributes.end(); it++ ) {
        if ( it->second->IsType<bool>()) {
            dict.setBoolForKey(it->first.c_str(), it->second->as<bool>());
        } else if ( it->second->IsType<int>()) {
            dict.setIntegerForKey(it->first.c_str(), it->second->as<int>());
        } else if ( it->second->IsType<float>()) {
            dict.setFloatForKey(it->first.c_str(), it->second->as<float>());
        } else if ( it->second->IsType<std::string>()) {
            dict.setStringForKey(it->first.c_str(), it->second->as<std::string>());
        }
    }
    if ( dict.saveRootSubDictToFile(fileName) ) {
        return true;
    }
    return false;
}

bool Options::load(const char *fileName)
{
    Dictionary dict;
    if ( dict.loadRootSubDictFromFile(fileName) ) {
        std::vector<std::string> keys = dict.getAllKeys();
        for ( int i=0; i<keys.size(); i++ ) {
            int keyType = dict.getTypeForKey(keys[i].c_str());
            if ( keyType == DD_Bool ) {
                addOption( keys[i], dict.getBoolForKey(keys[i].c_str()) );
            } else if ( keyType == DD_Int ) {
                addOption( keys[i], dict.getIntegerForKey(keys[i].c_str()) );
            } else if ( keyType == DD_Float ) {
                addOption( keys[i], dict.getFloatForKey(keys[i].c_str()) );
            } else if ( keyType == DD_String ) {
                addOption( keys[i], dict.getStringForKey(keys[i].c_str()) );
            } else {
                printf("[Options] bad type in dictionary for %s (%i)\n", keys[i].c_str(), keyType);
            }
        }
        return true;
    }
    return false;
}

void Options::setDefaults()
{
    // Clear out old options
    m_Attributes.empty();
    
    addOption("version", "0");
    addOption("h_multiThreading", true);
    
    addOption("r_resolutionX", 1280);
    addOption("r_resolutionY", 720);
    
    addOption("r_useShaders", true);
    addOption("r_deferred", true);
    addOption("r_fullScreen", false);
    addOption("r_debug", false);
    
    addOption("r_vSync", true);
    addOption("r_fsAA", false);
    addOption("r_fxAA", false);
    
    addOption("r_lighting2D", true);
    addOption("r_lighting3D", true);
    addOption("r_debugLights", false);
    addOption("r_debugShadows", false);
    addOption("r_lightRays", false);
    addOption("r_shadows", false);
    addOption("r_shadowMultitap", false);
    addOption("r_shadowNoise", false);
    addOption("r_renderFog", true);
    addOption("r_fogDensity", 0.75f);
    addOption("r_fogHeightFalloff", 0.25f);
    addOption("r_fogExtinctionFalloff", 20.0f);
    addOption("r_fogInscatteringFalloff", 20.0f);
    
    addOption("r_renderMap", false);
    
    addOption("r_sun", true);
    addOption("r_sunBlur", false);
    addOption("r_sunFlare", false);
    
    addOption("r_renderDOF", false);
    addOption("r_renderFisheye", false);
    addOption("r_renderVignette", false);
    addOption("r_renderCorrectGamma", false);
    addOption("r_renderToneMap", false);
    
    addOption("r_renderFlare", false);
    addOption("r_flareSamples", 5);
    addOption("r_flareDispersal", 0.3f);
    addOption("r_flareHaloWidth", 0.45f);
    addOption("r_flareChromaDistortionX", 0.01f);
    addOption("r_flareChromaDistortionY", 0.03f);
    addOption("r_flareChromaDistortionZ", 0.05f);
    addOption("r_flareThreshold", 0.75f);
    addOption("r_flareGain", 1.0f);
    
    addOption("r_renderSSAO", false);
    addOption("r_SSAOblur", 0);
    
    addOption("r_SSAOtotal_strength", 1.0f);
    addOption("r_SSAObase", 0.0f);
    addOption("r_SSAOarea", 0.008f);
    addOption("r_SSAOfalloff", 0.00001f);
    addOption("r_SSAOradius", 0.06f);
    
    addOption("r_renderEdge", false);
    addOption("r_edgeSobel", false);
    addOption("r_edgeBrightness", 1.0f);
    addOption("r_edgeThreshold", 0.05f);
    
    addOption("r_motionBlur", false);
    addOption("r_renderWireFrame", false);
    addOption("r_renderPoint", false);
    addOption("r_renderParticles", true);
    
    addOption("r_voxelCulling", true);
    
    addOption("r_grabCursor", false);
    
    addOption("e_worldLabels", false);
    addOption("e_gridRender", false);
    addOption("e_gridSnap", false);
    addOption("e_gridSize", 1.0f);
    
    addOption("i_invertLeftStickX", false);
    addOption("i_invertLeftStickY", false);
    addOption("i_invertRightStickX", false);
    addOption("i_invertRightStickY", false);
    
    addOption("a_playSFX", false);
    addOption("a_playMusic", false);
    addOption("a_playMenuSFX", false);
    addOption("a_volumeSFX", 1.0f);
    addOption("a_volumeMusic", 1.0f);
    addOption("a_volumeMenu", 0.3f);
    
    addOption("e_gridSize", 1.0f);
    addOption("e_gridSnap", false);
    
    addOption("n_networkEnabled", false);
    addOption("d_physics", false);
}
