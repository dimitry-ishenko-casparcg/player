from gi.repository import Nautilus, GObject
import subprocess

class CasparCGPlayer(GObject.GObject, Nautilus.MenuProvider):
    def _launch(self, menu, folder):
        path = folder.get_location().get_path()
        subprocess.Popen(["casparcg-player", path], start_new_session=True)

    def _make_item(self, files):
        item = Nautilus.MenuItem(
            name="CasparCGPlayer::Play",
            label="Play with CasparCG",
            icon="media-playback-start",
        )
        item.connect("activate", self._launch, files[0])
        return item

    def get_file_items(self, files):
        if len(files) != 1 or not files[0].is_directory(): return []
        else: return [self._make_item(files)]

    def get_background_items(self, current_folder):
        return [self._make_item([current_folder])]
