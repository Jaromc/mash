//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------------

#ifndef _MASH_H_
#define _MASH_H_

#include "MashCompileSettings.h"
#include "MashDevice.h"
#include "MashCreationParameters.h"
#include "MashEnum.h"

namespace mash
{
    /*!
        This is implimented by the user to become that application loop.
     
        Update() maybe called many times per frame or not at all depending
        on the frame rate.
     
        All rendering happens in Render(). Draw calls will only be valid from here.
    */
	class MashGameLoop : public MashReferenceCounter
	{
	public:
		MashGameLoop():MashReferenceCounter(){}
		virtual ~MashGameLoop(){}

        //! Initialise this loop and engine.
		/*!
			All scene creation should be done here including lighting, file loading, mesh
            creation, buffer creation, applying materials to meshes, etc... These things 
            can be done at runtime but performance may be affected.
         
            \return True to quit the app.
		*/
		virtual bool Initialise() = 0;

        //! Main application loop.
        /*!
             This function runs on a fixed update time.
         
			This maybe called many times from the device per frame depending on the frame rate. 
			All scene updating must be done here. Use MashSceneManager::UpdateScene() and pass
            in each scene graph that needs updating.
         
            You should do any application updating from here too.
            
			\param dt Fixed update time in seconds. Eg, 0.016 for 60fps.
            \return True to quit the app.
		*/
		virtual bool Update(float dt) = 0;

        //! Late update. Called once per frame before Render(). 
        /*!
            This function runs on a variable time and may be called many times
			before the next time Update() is called.
         
            If the physics library is used then all physics updates will be made before
            entering this function. From here you can grab final world positions and update 
            anything that needs to wait for the scene to be completely updated.
		*/
		virtual void LateUpdate(){};
		
        //! Rendering function.
        /*!
			Culling and rendering occurs here.
         
            MashTimer::GetFrameInterpolatorTime() is a function of the current frame time between 0.0 and 1.0 that
			interpolates forward for each render call between calls to Update().
			Update() maybe called many times per frame or skipped depending on the current frame rate. 
            The interpolator time tells certain objects how far they need to interpolate till the
            next time Update() is called. This provides smooth transformations between updates.
         
            From here you should call MashSceneManager::CullScene() for each scene graph you want
            rendered then MashSceneManager::DrawScene() to draw everything.
		*/
		virtual void Render() = 0;

		//! Called when the pause state is entered in MashDevice::SetGameState().
		/*!
			When the pause state is entered only this function will be called.

			Internally, only the input manager will be updated. Other modules such as the physics and
			scene managers are not updated any further till the pause state is left.

			You can continue to call the scenes culling and draw functions from here to
			display the scene in a paused state while rendering a GUI over the top.

			\return True to quit the app.
		*/
		virtual bool Pause(){return false;}

        //! Called when another window other than this app takes focus.
        /*!
            From here you could start to put the app to sleep for a period of time
            to free the CPU for other windows.
        */
        virtual void WindowFocusLost(){}

        //! Called when this window regains focus.
        virtual void WindowFocusRegained(){}
	};

    //! Called to create the device.
	_MASH_EXPORT MashDevice* CreateDevice (sMashDeviceSettings &settings);
}

#endif
