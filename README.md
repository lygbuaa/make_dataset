# ffmpeg tools
### ffmpeg clip video
`ffmpeg -i input.mkv -ss 0:13 -t 6 -c:v libx265 output.mkv` #start from 0:13, clip 6secs, H265 output

### 支持的像素格式
`ffmpeg -pix_fmts`

### 抽帧 This will extract one video frame per 20 second from the video and will output them in files named foo-001.jpeg,
### foo-002.jpeg, etc. Images will be rescaled to fit the new WxH values.
### Incompatible pixel format 'uyvy422' for codec 'mjpeg', auto-selecting format 'yuvj420p'
`ffmpeg -i sample.mkv -r 0.05 -s 3840x2160 -f image2 -pix_fmt yuvj444p sample_%03d.jpg`

### jpg to nv12
`ffmpeg -i sample_001.jpg  -pix_fmt nv12 sample_001.yuv`

### jpg to h265
`ffmpeg -framerate 1 -i sample_001.jpg -c:v libx265 -crf 15 -preset ultrafast -pix_fmt nv12 -c:a copy sample_frame_001.mkv`

### nv12 to h265
`ffmpeg -framerate 1 -f rawvideo -pix_fmt nv12 -video_size 3840x2160 -i sample_001.yuv -c:v libx265 -crf 15 -preset ultrafast -pix_fmt nv12 -c:a copy sample_frame_nv12.mkv`

### uyvy to nv12
`ffmpeg -f rawvideo -pix_fmt uyvy422 -video_size 3840x2160 -i 8m.uyvy  -pix_fmt nv12 8m.yuv`

### nv12 to uyvy
`ffmpeg -f rawvideo -pix_fmt nv12 -video_size 3840x2160 -i 8m.nv12  -pix_fmt uyvy422 8m.yuv`

### dump video information
`ffmpeg -i sample_h265.mkv -hide_banner`

### convert h265 frame to nv12
`ffmpeg -i sample_h265.mkv  -pix_fmt nv12 sample.yuv`

# strace tools 
### binary needs full path
`/usr/bin/strace  /mnt/c/work/github/make_dataset/build/make_offline_data_bin`

# gdb tools
### start debug
`/usr/bin/gdb /mnt/c/work/github/make_dataset/build/make_offline_data_bin`

### set breakpoint at line make_offline_data.cc:8
`b make_offline_data.cc:8`
### run program
`r`
### print variable
`p argc`

