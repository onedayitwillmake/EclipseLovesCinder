/*
 *  OpenNIThreadRunner.cpp
 *  CinderRibbons
 *
 *	This file represents a simple thread which continuously updates OpenNI
*/
/*
/*
  ####  #####  ##### ####    ###  #   # ###### ###### ##     ##  #####  #     #      ########    ##    #  #  #####
 #   # #   #  ###   #   #  #####  ###    ##     ##   ##  #  ##    #    #     #     #   ##   #  #####  ###   ###
 ###  #   #  ##### ####   #   #   #   ######   ##   #########  #####  ##### ##### #   ##   #  #   #  #   # #####
 --
 Mario Gonzalez
 CinderOpenNIBarebones - https://github.com/onedayitwillmake/CinderOpenNIBarebones/blob/master/CinderOpenNI.cpp
 */

#include "CinderOpenNI.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

class OpenNIThreadRunner
{
public:
    OpenNIThreadRunner()
	: m_stoprequested(false)
	{

	}

    ~OpenNIThreadRunner()
    {
        m_stoprequested = true;
		m_thread->join();
    }

	void go()
	{
		assert(!m_thread);
        m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&OpenNIThreadRunner::updateKinect, this)));
	}

    void stop()
    {
        assert(m_thread);
        m_stoprequested = true;
        m_thread->join();
    }

private:
    volatile bool m_stoprequested;
    boost::shared_ptr<boost::thread> m_thread;
    boost::mutex m_mutex;

    // Update the kincet as fast as possible
    void updateKinect()
	{
		int iteration = 0;

		while (!m_stoprequested)
		{
			XnStatus nRetVal = XN_STATUS_OK;
			CinderOpenNISkeleton *skeleton = CINDERSKELETON;

			// nRetVal = skeleton->mContext.WaitOneUpdateAll( skeleton->mDepthGenerator );
			nRetVal = skeleton->mContext.WaitAndUpdateAll();

			if( nRetVal != XN_STATUS_OK ) {
				printf("WaitAndUpdateAll failed: %s\n", xnGetStatusString(nRetVal));
				continue;
			}

			// Get depthgenerator
			nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_DEPTH, skeleton->mDepthGenerator);
			if( nRetVal != XN_STATUS_OK ) {
				printf("FindExistingNode failed: %s\n", xnGetStatusString(nRetVal));
				continue;
			}

			// Cannot retrieve FPS or depthGenerator
			if(!&skeleton->xnFPS) {
				std::cout << "(Oneday) OpenNIThreadRunner Not ready!" << std::endl;
				continue;
			}

			skeleton->mDepthGenerator.GetMetaData( skeleton->mDepthMD );
			skeleton->mUserGenerator.GetUserPixels(0, skeleton->mSceneMD);
			skeleton->setDepthSurface();

			++iteration;
		}
	}
};


