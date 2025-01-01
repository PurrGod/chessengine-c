from pytubefix import Playlist
from datetime import timedelta

def get_playlist_durations(playlist_url):
    # Load the playlist
    playlist = Playlist(playlist_url)
    
    total_seconds = 0
    
    # Iterate through each video in the playlist
    for video in playlist.videos:
        print(f"Video ID: {video.video_id}, Title: {video.title}, Duration: {video.length}")
        try:
            # Get the duration of the video in seconds
            duration = video.length
            if duration is not None:
                total_seconds += duration
            else:
                print(f"Skipping video: {video.title} (duration not available)")
        except Exception as e:
            print(f"Skipping video: {video.title} (error: {e})")
    
    # Convert total seconds to minutes and hours
    total_duration = timedelta(seconds=total_seconds)
    return total_duration

if __name__ == "__main__":
    # Check if a URL is provided as a command-line argument
    import sys
    if len(sys.argv) != 2:
        print("Usage: python ./playlisttime.py <PLAYLIST_URL>")
        sys.exit(1)
    
    # Get the URL from the command-line argument
    playlist_url = sys.argv[1]
    
    # Calculate and print the total duration
    total_duration = get_playlist_durations(playlist_url)
    print(f"Total duration of all videos: {total_duration}")