// dc-ctl.cpp
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <dc1394/dc1394.h>
#include <libraw1394/raw1394.h>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include "config.h"


int featureMin(const dc1394featureset_t &features, dc1394feature_t feature)
{
    return features.feature[feature - DC1394_FEATURE_MIN].min;
}

int featureMax(const dc1394featureset_t &features, dc1394feature_t feature)
{
    return features.feature[feature - DC1394_FEATURE_MIN].max;
}

void setFeature(dc1394camera_t *camera, const dc1394featureset_t &features, 
        dc1394feature_t feature, const std::string &valueStr)
{
    using boost::lexical_cast;
    const uint32_t MIN =  featureMin(features, feature);
    const uint32_t MAX =  featureMax(features, feature);
    dc1394error_t camerr;
    
    if (valueStr == "auto")
    {
        camerr = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_AUTO);
        if (camerr != DC1394_SUCCESS)
            throw std::runtime_error("libdc1394 error: this should be more verbose");
    }
    else
    {
        dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        const uint32_t value =  lexical_cast<int>(valueStr);

        camerr = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        if (camerr != DC1394_SUCCESS)
            throw std::runtime_error("libdc1394 error: this should be more verbose");
        if (value >=  MIN and value <= MAX)
            dc1394_feature_set_value(camera, feature, value);
        else
            std::cerr << "error: value must be in range [" << MIN <<  "," << MAX << "], ignoring\n"; 
    }
}


static std::map<std::string, dc1394feature_t> FEATURE_MAP;

void initFeatureMap()
{
    if (FEATURE_MAP.empty())
    {
        FEATURE_MAP["brightness"] = DC1394_FEATURE_BRIGHTNESS;
        FEATURE_MAP["auto-exposure"] = DC1394_FEATURE_EXPOSURE; 
        FEATURE_MAP["sharpness"] = DC1394_FEATURE_SHARPNESS;
        FEATURE_MAP["whitebalance"] = DC1394_FEATURE_WHITE_BALANCE;
        FEATURE_MAP["saturation"] = DC1394_FEATURE_SATURATION;
        FEATURE_MAP["gamma"] = DC1394_FEATURE_GAMMA;
        FEATURE_MAP["shutter-time"] = DC1394_FEATURE_GAMMA;
        FEATURE_MAP["gain"] = DC1394_FEATURE_GAIN;
    }
}

/// converts from feature name (string) to the corresponding constant
dc1394feature_t featureNameToConstant(const std::string &featureName)
{
    initFeatureMap();

    return FEATURE_MAP[featureName];
}

std::string getFeatureValue(const dc1394featureset_t &features, dc1394feature_t feature, dc1394camera_t * camera)
{
    // if it's auto, we don't care about the value
    if (features.feature[feature - DC1394_FEATURE_MIN].current_mode == DC1394_FEATURE_MODE_AUTO)
        return std::string("auto");
    else
    {
        unsigned int value;
        std::ostringstream result;
        dc1394error_t error = dc1394_feature_get_value(camera, feature, &value);
        if (error != DC1394_SUCCESS)
            throw std::runtime_error("libdc1394 error: could not get value");
        result << value;
        return result.str();
    }
}

/// pass by reference because we're expecting multiple values, alternately could return a vector
std::string getWhiteBalance(const dc1394featureset_t &features, dc1394camera_t * camera)
{
    if (features.feature[DC1394_FEATURE_WHITE_BALANCE - DC1394_FEATURE_MIN].current_mode == DC1394_FEATURE_MODE_AUTO)
        return std::string("auto");
    else
    {
        unsigned int valueBU, valueRV;
        dc1394error_t error = dc1394_feature_whitebalance_get_value(camera, &valueBU, &valueRV);
        if (error != DC1394_SUCCESS)
            throw std::runtime_error("libdc1394 error: could not get value");
        std::ostringstream result;
        result << valueBU << "," << valueRV;
        return result.str();
    }
}

void printFeatureValue(const std::string &featureName, dc1394camera_t *camera)
{
    dc1394featureset_t features;
    dc1394error_t camerr = dc1394_feature_get_all(camera, &features);
    if (camerr != DC1394_SUCCESS)
        throw std::runtime_error("libdc1394 error: this should be more verbose");

    // special case, whitebalance has multiple values
    if (featureName == "whitebalance")
    {
        std::cout << featureName << "=" 
            << getWhiteBalance(features,camera)
            << std::endl;
    }
    else
    {
        std::cout << featureName << "=" 
            << getFeatureValue(features, featureNameToConstant(featureName), camera)
            << std::endl;
    }
}

void printAllFeatureValues(dc1394camera_t *camera)
{
    using std::map;
    using std::string;
    initFeatureMap();
    for (map<string, dc1394feature_t>::iterator iter = FEATURE_MAP.begin(); 
            iter != FEATURE_MAP.end(); ++iter)
                printFeatureValue(iter->first, camera);
}


void saveSettings(const std::string &filename, dc1394camera_t * camera)
{
    dc1394featureset_t features;
    dc1394error_t camerr = dc1394_feature_get_all(camera, &features);
    if (camerr != DC1394_SUCCESS)
        throw std::runtime_error("libdc1394 error: this should be more verbose");

    std::cout << "Saving settings to " << filename << std::endl;

    std::ofstream fout;
    try 
    {
        fout.open(filename.c_str());
        fout << "camera=" << std::hex << camera->guid << "\n" << std::dec;
        fout << "brightness=" << getFeatureValue(features, DC1394_FEATURE_BRIGHTNESS, camera) << "\n";
        fout << "auto-exposure=" << getFeatureValue(features, DC1394_FEATURE_EXPOSURE, camera) << "\n";
        fout << "sharpness=" << getFeatureValue(features, DC1394_FEATURE_SHARPNESS, camera) << "\n";
        fout << "whitebalance=" << getWhiteBalance(features, camera) << "\n";
        fout << "saturation=" << getFeatureValue(features, DC1394_FEATURE_SATURATION, camera) << "\n";
        fout << "gamma=" << getFeatureValue(features, DC1394_FEATURE_GAMMA, camera) << "\n";
        fout << "shutter-time=" << getFeatureValue(features, DC1394_FEATURE_SHUTTER, camera) << "\n";
        fout << "gain=" << getFeatureValue(features, DC1394_FEATURE_GAIN, camera) << "\n";
        fout.close();
    }
    catch (const std::exception& e)  // catch it here so that we close file
    {
        std::cerr << "error: " << e.what() << "\n";
    }
    fout.close();
}

std::string featureHelp(const dc1394featureset_t &features, dc1394feature_t feature)
{
    using std::string;
    using boost::lexical_cast;
    string helpStr;

    switch (feature)
    {
        case DC1394_FEATURE_BRIGHTNESS:
            helpStr = "brightness [";
            break;
        case DC1394_FEATURE_EXPOSURE:
            helpStr = "auto-exposure [";
            break;
        case DC1394_FEATURE_SHARPNESS:
            helpStr = "sharpness [";
            break;
        case DC1394_FEATURE_WHITE_BALANCE:
            helpStr = "Blue/U Red/V [";
            helpStr += lexical_cast<string>(features.feature[feature - DC1394_FEATURE_MIN].min);
            helpStr += ",";
            helpStr += lexical_cast<string>(features.feature[feature - DC1394_FEATURE_MIN].max) + "]"; 
            helpStr += " [" + lexical_cast<string>(features.feature[feature - DC1394_FEATURE_MIN].min);
            helpStr += ",";
            helpStr += lexical_cast<string>(features.feature[feature - DC1394_FEATURE_MIN].max) + "]"; 
            return helpStr;
        case DC1394_FEATURE_SATURATION:
            helpStr = "saturation [";
            break;
        case DC1394_FEATURE_GAMMA:
            helpStr = "gamma [";
            break;
        case DC1394_FEATURE_SHUTTER:
            helpStr = "shutter-time [";
            break;
        case DC1394_FEATURE_GAIN:
            helpStr = "gain (db) [";
            break;
        default:
            std::cerr << "Unknown feature " << feature;
            break;
    }

    helpStr += lexical_cast<string>(features.feature[feature - DC1394_FEATURE_MIN].min);
    helpStr += ",";
    helpStr += lexical_cast<string>(features.feature[feature - DC1394_FEATURE_MIN].max) + "]"; 
    return helpStr;
}


void cleanup(dc1394_t * dc1394, dc1394camera_t *camera, dc1394camera_list_t *cameras)
{
    if (camera != 0)
        free(camera);
    if (cameras != 0)
        dc1394_camera_free_list(cameras);
    if (dc1394 != 0)
        dc1394_free(dc1394);
}


int run(int argc, char *argv[])
{
    /// turn off error logging from library as this conflicts with our output
    void *user_data = 0;
    dc1394_log_register_handler(DC1394_LOG_ERROR, NULL, user_data);
    dc1394_t * dc1394 = 0; 
    dc1394camera_t *camera = 0;
    dc1394camera_list_t *cameras = 0;

    try 
    {
        namespace po = boost::program_options;
        using std::string;
        using boost::lexical_cast;
        using boost::tokenizer;
        using boost::char_separator;
        po::options_description desc;
        // Add genenic, non-dc1394 dependent options first
        desc.add_options()
            ("help,h", "produce help message, which will contain more device-specific information if a camera is connected")
            ("version", "produce version")
            ("camera,c", po::value<string>()->default_value("0"), "guid of camera number to use (0 is first camera on bus)")
            ("config,C", po::value<string>(), "path of file with configuration presets")
            ("list-features,l", po::bool_switch(), "print available features for this camera")
            ("list-settings,L", po::bool_switch(), "print current settings for this camera")
            ("save,x", po::value<string>(), "save current camera settings to the specified filename")
            ;

        // make sure raw1394 is loaded and read/writeable
        raw1394handle_t tmp_handle = raw1394_new_handle();
        if (tmp_handle == NULL) 
        {
            throw std::runtime_error("Warning: could not get a handle to your IEEE1394 card.\n\n"
                    "Please check that:\n- the card is present\n- the IEEE1394 modules (ieee1394,"
                    "ohci1394,\n     raw1394 and video1394) are loaded\n- you have read/write "
                    "permissions on the\n     /dev/raw1394 and /dev/video1394 devices.");
        }
        else
            raw1394_destroy_handle(tmp_handle);
        
        po::variables_map vm;

        // get camera information first to have valid ranges
        dc1394error_t camerr;
        dc1394 = dc1394_new();
        if (dc1394 == 0)
        {
            // no dc1394 module present, so if we asked for help 
            // we'll just show that
            po::store(po::parse_command_line(argc, argv, desc), vm);
            po::notify(vm);
            if (vm.count("help") or argc == 1)  // no args
            {
                std::cout << desc << "\n";
                return 0;
            }
            else if (vm.count("version"))
            {
                std::cout << "dc-ctl " << PACKAGE_VERSION << "\n";
                return 0;
            }
            else
            {
                std::cerr << "No dc1394 module present\n";
                return 1;
            }
        }
        camerr = dc1394_camera_enumerate(dc1394, &cameras);
        if (camerr != DC1394_SUCCESS)
            throw std::runtime_error("libdc1394 error: this should be more verbose");
        if (cameras->num == 0)
            throw std::runtime_error("libdc1394 error: no camera found on bus");

        camera = dc1394_camera_new_unit(dc1394, cameras->ids[0].guid,
                cameras->ids[0].unit);

        dc1394featureset_t features;
        camerr = dc1394_feature_get_all(camera, &features);
        if (camerr != DC1394_SUCCESS)
            throw std::runtime_error("libdc1394 error: this should be more verbose");

        // FIXME: right now we can only use camera 0. But we can't display the valid ranges in the help if we
        // don't know ahead of time which camera to use.

        // using strings so that value can be "auto", set default value so that if they're not 
        // given values we can print out their current values
        desc.add_options()
            ("brightness,b", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_BRIGHTNESS).c_str())
            ("auto-exposure,e", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_EXPOSURE).c_str())
            ("sharpness,s", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_SHARPNESS).c_str())
            ("whitebalance,w", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_WHITE_BALANCE).c_str())
            ("saturation,S", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_SATURATION).c_str())
            ("gamma,g", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_GAMMA).c_str())
            ("shutter-time,t", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_SHUTTER).c_str())
            ("gain,G", po::value<string>()->implicit_value(""), featureHelp(features, DC1394_FEATURE_GAIN).c_str())
            ;

        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("version"))
        {
            std::cout << PACKAGE_VERSION << "\n";
            cleanup(dc1394, camera, cameras);
            return 0;
        }
        else if (vm.count("help") or argc == 1)  // no args
        {
            std::cout << desc << "\n";
            cleanup(dc1394, camera, cameras);
            return 0;
        }

        if(vm.count("save"))
        {
            saveSettings(vm["save"].as<string>(), camera);
            cleanup(dc1394, camera, cameras);
            return 0;
        }

        if(vm.count("config"))
        {
            std::ifstream configFile(vm["config"].as<string>().c_str());
            if (not configFile.good())
            {
                configFile.close();
                throw std::runtime_error("Could not open file " + vm["config"].as<string>()); 
            }
            store(parse_config_file(configFile, desc), vm);
        }

        if (vm["camera"].as<string>() != "0") // using non-default camera id
        {
            // find right guid
            bool matchedGUID = false;
            for (unsigned i = 0; i < cameras->num and not matchedGUID; ++i)
            {
                std::stringstream GUIDInHex;
                GUIDInHex << std::hex << cameras->ids[i].guid;
                if (GUIDInHex.str() == vm["camera"].as<string>())
                {
                    camera = dc1394_camera_new_unit(dc1394, cameras->ids[i].guid,
                            cameras->ids[i].unit);
                    matchedGUID = true;
                }
            }
            if (not matchedGUID)
                throw std::runtime_error("could not find camera with guid " + 
                        lexical_cast<string>(vm["camera"].as<string>()));


            camerr = dc1394_feature_get_all(camera, &features);
            if (camerr != DC1394_SUCCESS)
                throw std::runtime_error("libdc1394 error: this should be more verbose");
        }

        if (vm["list-features"].as<bool>())
        {
            camerr = dc1394_feature_print_all(&features, stdout);
            // FIXME: actually look at the error value
            if (camerr != DC1394_SUCCESS)
                throw std::runtime_error("libdc1394 error: could not print features");
            std::cout << std::endl;
            cleanup(dc1394, camera, cameras);
            return 0;
        }

        if (vm["list-settings"].as<bool>())
        {
            printAllFeatureValues(camera);
            cleanup(dc1394, camera, cameras);
            return 0;
        }

        /// check if we're just printing a feature's current value
        bool gotFeature = false;
        for (po::variables_map::iterator iter = vm.begin(); iter != vm.end(); ++iter)
        {
            /// parameter has been given with no value
            if (iter->second.value().type() == typeid(string))
                if (iter->second.as<string>().empty())
                {
                    gotFeature = true;
                    printFeatureValue(iter->first, camera);
                }
        }

        /// this is all we're supposed to do, time to exit
        if (gotFeature)
        {
            cleanup(dc1394, camera, cameras);
            return 0;
        }

        if(vm.count("brightness"))
        {
            std::cout << "Setting brightness=" << vm["brightness"].as<string>() << std::endl;
            setFeature(camera, features, DC1394_FEATURE_BRIGHTNESS, vm["brightness"].as<string>());
        }

        if(vm.count("auto-exposure"))
        {
            std::cout << "Setting auto-exposure=" << vm["auto-exposure"].as<string>() << std::endl;
            setFeature(camera, features, DC1394_FEATURE_EXPOSURE, vm["auto-exposure"].as<string>());
        }

        if(vm.count("sharpness"))
        {
            std::cout << "Setting sharpness=" << vm["sharpness"].as<string>() << std::endl;
            setFeature(camera, features, DC1394_FEATURE_SHARPNESS, vm["sharpness"].as<string>());
        }

        if (vm.count("whitebalance"))
        {
            // FIXME: Hack because i don't know how to have defaults for vector program options
            string u_b_string;
            string v_r_string;
            bool autoWhiteBalance = false;

            // either auto or values
            if (vm["whitebalance"].as<string>() == "auto")
            {
                camerr = dc1394_feature_set_mode(camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_AUTO);
                if (camerr != DC1394_SUCCESS)
                    throw std::runtime_error("libdc1394 error: this should be more verbose");
                autoWhiteBalance = true;
                std::cout << "Setting whitebalance=auto\n";
            }
            else // need to parse tokens
            {
                char_separator<char> sep(",");
                tokenizer< char_separator<char> > tokens(vm["whitebalance"].as<string>(), sep);
                int count = 0;
                BOOST_FOREACH(string t, tokens)
                {
                    switch (count)
                    {
                        case 0:
                            u_b_string = t;
                            break;
                        case 1:
                            v_r_string = t;
                            break;
                        default:
                            std::cerr << "error: white balance takes two arguments, Blue/U and Red/V [int int]\n";
                            cleanup(dc1394, camera, cameras);
                            return 1;
                    }
                    ++count;
                }
            }

            if (not autoWhiteBalance)
            {
                const int MIN_WHITE_BALANCE = featureMin(features, DC1394_FEATURE_WHITE_BALANCE);
                const int MAX_WHITE_BALANCE = featureMax(features, DC1394_FEATURE_WHITE_BALANCE);
                const int u_b = lexical_cast<int>(u_b_string); // convert to ints
                const int v_r = lexical_cast<int>(v_r_string); // convert to ints

                if (u_b >= MIN_WHITE_BALANCE and 
                        u_b <= MAX_WHITE_BALANCE and 
                        v_r >= MIN_WHITE_BALANCE and 
                        v_r <= MAX_WHITE_BALANCE)
                {
                    std::cout << "Setting white balance=Blue/U=" << u_b << ", Red/V=" << v_r << "\n";
                    camerr = dc1394_feature_set_mode(camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_MANUAL);
                    camerr = dc1394_feature_whitebalance_set_value(camera, u_b, v_r);
                    if (camerr != DC1394_SUCCESS)
                        throw std::runtime_error("libdc1394 error: this should be more verbose");
                }
                else
                {
                    std::cerr << "error: whitebalance values must be in range [" 
                        << MIN_WHITE_BALANCE << "," << MAX_WHITE_BALANCE << "]" << std::endl;
                    cleanup(dc1394, camera, cameras);
                    return 1;
                }
            }
        }

        if (vm.count("saturation"))
        {
            std::cout << "Setting saturation=" << vm["saturation"].as<string>() << std::endl;
            setFeature(camera, features, DC1394_FEATURE_SATURATION, vm["saturation"].as<string>());
        }

        if (vm.count("gamma"))
        {
            std::cout << "Setting gamma=" << vm["gamma"].as<string>() << std::endl;
            setFeature(camera, features, DC1394_FEATURE_GAMMA, vm["gamma"].as<string>());
        }

        if (vm.count("shutter-time"))
        {
            std::cout << "Setting shutter-time=" << vm["shutter-time"].as<string>() << std::endl;
            setFeature(camera, features, DC1394_FEATURE_SHUTTER, vm["shutter-time"].as<string>());
        }

        if (vm.count("gain"))
        {
            std::cout << "Setting gain=" << vm["gain"].as<string>() << std::endl;
            setFeature(camera, features, DC1394_FEATURE_GAIN, vm["gain"].as<string>());
        }
    }
    catch (const std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";
        cleanup(dc1394, camera, cameras);
        return 1;
    }
    catch (...) 
    {
        // FIXME: is this possible?
        std::cerr << "Exception of unknown type!\n";
        cleanup(dc1394, camera, cameras);
        return 1;
    }

    cleanup(dc1394, camera, cameras);
    return 0;
}

// FIXME: i should grab the error return codes from all the libdc calls and check them
int main(int argc, char *argv[])
{
    return run(argc, argv);
}

