(function () {
    console.info("Touch detector script starting");
    
    var MAX_AVATAR_DISTANCE = 3.0;
    var JOINT_COLLISION_DISTANCE = 0.2;
    
    var touchingJoints = ["RightHand", "LeftHand"];
    var touchableJoints = [];
    var jointNames = {}; // User friendly joint names
    
    
    var FT_LEFT_EAR  = 0x00;
    var FT_RIGHT_EAR = 0x01;
    var FT_TAIL      = 0x02;

    var HAPTIC_STRENGTH = 0.9;
    var HAPTIC_DURATION = 125;
    
    function jointTouched(avatar, joint_touched, touched_with) {
        var name = jointNames[joint_touched];
        var part = FT_LEFT_EAR;
        
        console.log("Avatar " + avatar.sessionDisplayName + " touched " + joint_touched + " (" + name + ") with " + touched_with);
        
        if ( name == "RightEar" ) {
            part = FT_RIGHT_EAR;
        } else if ( name == "LeftEar" ) {
            part = FT_LEFT_EAR;
        } else if ( name == "Tail" ) {
            part = FT_TAIL;
        } else {
            console.warn("Don't know what to do with " + name);
            return;
        }
            
        var deviceId = Controller.findDevice("FurTouch");
        
        
        if ( deviceId != 65535 ) {
            console.debug("Sending pulse to device " + deviceId + ", to part " + part);
            Controller.triggerHapticPulseOnDevice(deviceId, HAPTIC_STRENGTH, HAPTIC_DURATION, part);
        } else {
            console.warn("FurTouch device not found");
        }
    }
    
    function checkTouch() {
        var nearbyAvatars = AvatarList.getAvatarsInRange(MyAvatar.position, MAX_AVATAR_DISTANCE);
        var jointPositions = [];
        
        // Get the positions of all our joints
        touchableJoints.forEach(function(joint) {
            jointPositions[joint] = MyAvatar.getJointPosition(joint);
        });
        
        // Check if anything collided
        nearbyAvatars.forEach(function(identifier) {
            var av = AvatarList.getAvatar(identifier);
            
            touchingJoints.forEach(function(touching_joint) {
                var pos = av.getJointPosition(touching_joint);
                
                touchableJoints.forEach(function(touchable_joint) {
                    if ( Vec3.distance( pos, jointPositions[touchable_joint]) <= JOINT_COLLISION_DISTANCE ) {
                        jointTouched( av, touchable_joint, touching_joint );
                    }
                });
            });
            
        });
    }
    
   
    function init() {
        MyAvatar.getJointNames().forEach(function(name) {
            if ( name.indexOf("flow_") == 0 ) {
                console.log("Joint: " + name);
                
                var index = MyAvatar.getJointIndex(name);
                
                console.log("Index: " + index);
                
                touchableJoints.push(index);
                
                // Convert the joint name to something more readable:
                // flow_Tail_1 => Tail
                var short_name = name.substring(5); // Remove flow_
                var underscore_pos = short_name.indexOf("_");
                
                if ( underscore_pos >= 0 ) {
                    short_name = short_name.substring(0, underscore_pos);
                }
                
                console.log("Joint " + name + " => " + short_name);
                jointNames[index] = short_name;
            }
        });
    }
    
    Script.update.connect( checkTouch );
    MyAvatar.skeletonModelURLChanged.connect( init );
    init();
}());
