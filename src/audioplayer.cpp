#include "audioplayer.h"
#include "jgfxhandler.h"
#include "jautolock.h"
#include "jdebug.h"

AudioPlayer::AudioPlayer(std::string file)
{
	_file = file;
	_surface = NULL;
	_is_closed = false;

	IDirectFB *directfb = (IDirectFB *)jgui::GFXHandler::GetInstance()->GetGraphicEngine();

	if (directfb->CreateVideoProvider(directfb, _file.c_str(), &_provider) != DFB_OK) {
		JDEBUG(JWARN, "Cannot load audio \"%s\"\n", _file.c_str());
		_provider = NULL;

		return;
	}

	DFBSurfaceDescription sdsc;
	DFBStreamDescription mdsc;

	_provider->SetPlaybackFlags(_provider, DVPLAY_NOFX);
	_provider->GetSurfaceDescription(_provider, &sdsc);
	_provider->GetStreamDescription(_provider, &mdsc);

	sdsc.flags  = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS);
	sdsc.width  = 4;
	sdsc.height = 4;
	sdsc.caps   = DSCAPS_SYSTEMONLY;

	directfb->CreateSurface(directfb, &sdsc, &_surface);
}

AudioPlayer::~AudioPlayer()
{
	jthread::AutoLock lock(&_mutex);

	if (_is_closed == true) {
		return;
	}

	_is_closed = true;

	if (_provider != NULL) {
		_provider->Release(_provider);
		_provider = NULL;
	}

	if (_surface != NULL) {
		_surface->Release(_surface);
	}
}

void AudioPlayer::Play()
{
	jthread::AutoLock lock(&_mutex);

	if (_provider != NULL) {
		_provider->PlayTo(_provider, (IDirectFBSurface *)_surface, NULL, NULL, NULL);

		usleep(500000);
	}
}
