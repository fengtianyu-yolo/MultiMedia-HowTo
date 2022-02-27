//
//  ViewController.swift
//  MultiMedia_Howto
//
//  Created by fengtianyu on 2022/2/27.
//

import Cocoa

class ViewController: NSViewController {

    let buttonSize = NSSize(width: 80, height: 30)
    let frameSize = NSSize(width: 320, height: 240)
    
    var button: NSButton?
    
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
        
        view.window?.center()
        
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }
    
    // MARK: - Event Action
    
    @objc func buttonClicked() {
        isRecoding = !isRecoding
        
        if isRecoding {
            // 更新按钮的状态
            button?.title = "停止录音"
            
            // 更新当前录制状态未录制中
            updateRecodeState(1)
            
            // 开启子线程，进行录音
            recordThread = Thread(target: self, selector: #selector(recordAction), object: nil)
            recordThread?.start()
        } else {
            button?.title = "开始录音"
            
            updateRecodeState(0)
        }
    }

    @objc func recordAction() {
        record()
    }

}

