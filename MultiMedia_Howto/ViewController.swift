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

    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.view.setFrameSize(frameSize)
        self.view.needsDisplay = true

        button = NSButton(title: "录音", target: self, action: #selector(buttonClicked))
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
        greeting()
    }


}

