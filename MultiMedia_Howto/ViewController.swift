//
//  ViewController.swift
//  MultiMedia_Howto
//
//  Created by fengtianyu on 2022/2/27.
//

import Cocoa

class ViewController: NSViewController {

    let buttonSize = NSSize(width: 80, height: 44)
    let frameSize = NSSize(width: 320, height: 240)
    
    var button: NSButton?
    var recordVideoButton: NSButton?
    var isRecodingVideo = false
    
    var isRecoding = false
    var recordThread: Thread?

    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.view.setFrameSize(frameSize)
        self.view.needsDisplay = true

        button = NSButton(title: "开始录音", target: self, action: #selector(buttonClicked))
        button?.frame = NSRect(x: (frameSize.width-buttonSize.width)/2, y: (frameSize.height-buttonSize.height)/2, width: buttonSize.width, height: buttonSize.height)
        button?.setButtonType(.pushOnPushOff)
        if let btn = button {
            view.addSubview(btn)
        }
        
        recordVideoButton = NSButton(title: "⏸", target: self, action: #selector(recordVideoAction))
        recordVideoButton?.frame = NSRect(x: (frameSize.width-buttonSize.width)/2, y: (frameSize.height-buttonSize.height)/2 + buttonSize.height + 40, width: buttonSize.width, height: buttonSize.height)
        recordVideoButton?.setButtonType(.pushOnPushOff)
        if let btn = recordVideoButton {
            view.addSubview(btn)
        }
        
        view.window?.center()
        
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }
    
    // MARK: - Video Event Actions
    
    @objc func recordVideoAction() {
        isRecodingVideo = !isRecodingVideo
        
        if isRecodingVideo {
            recordVideoButton?.title = "⏺"
            
            update_video_recording_status(1);
            
            // 开启子线程，进行录制
            recordThread = Thread(target: self, selector: #selector(startRecordingVideo), object: nil)
            recordThread?.start()
        } else {
            recordVideoButton?.title = "⏸"
            update_video_recording_status(0);
            
            recordThread?.cancel()
        }
    }
    
    @objc func startRecordingVideo() {
        record_video();
    }
    
    // MARK: - Audio Actions
    
    @objc func buttonClicked() {
        isRecoding = !isRecoding
        
        if isRecoding {
            // 更新按钮的状态
            button?.title = "停止录音"
            
            // 更新当前录制状态未录制中
            updateRecodeState(1)
            updateCaptureState(1)
            
            // 开启子线程，进行录音
            recordThread = Thread(target: self, selector: #selector(recordAction), object: nil)
            recordThread?.start()
        } else {
            button?.title = "开始录音"
            
            updateRecodeState(0)
            updateCaptureState(0)
        }
    }

    @objc func recordAction() {
        // 采集和编码
//        record()
        
        // 音频采集
        capture()
        
    }

}

