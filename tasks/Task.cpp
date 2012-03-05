/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

using namespace mr_control;

Task::Task(std::string const& name, TaskCore::TaskState initial_state)
    : TaskBase(name, initial_state)
//Task::Task(std::string const& name) //needs_configuration
//    : TaskBase(name)
    , MRconnector(0), MRrobot(0)
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine, TaskCore::TaskState initial_state)
    : TaskBase(name, engine, initial_state)
//Task::Task(std::string const& name, RTT::ExecutionEngine* engine) //needs_configuration
//    : TaskBase(name, engine)
    , MRconnector(0), MRrobot(0)
{
}

Task::~Task()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook()
{
    if (! TaskBase::configureHook())
        return false;
    
    ArArgumentBuilder *MRarguments = new ArArgumentBuilder();
    
    MRarguments->add("-robotPort");
    MRarguments->add(_serial_port.get().c_str());
    
    cout<<"Aria_Task: List of Parameters: "<<MRarguments->getFullString()<<endl;
    
    MRparser = new ArArgumentParser(MRarguments);
    MRparser->loadDefaultArguments();
    MRparser->log();
    
    if(!MRparser->checkHelpAndWarnUnparsed())
        cout<<"Aria_Task: Unparsed Arguments found"<<endl;
        
    cout<<"Aria_Task: Used Parameters "<<*MRparser->getArgv()<<endl;
    
    delete MRarguments;
    MRarguments = 0;
    
    return true;
}
bool Task::startHook()
{
    if (! TaskBase::startHook())
        return false;

    // Initialise Aria
    Aria::init();
    
    cout<<"Aria_Task: Initialised"<<endl;
    
    MRrobot = new ArRobot("", true, false);
    MRconnector = new ArRobotConnector(MRparser, MRrobot);
    
    //cout<<"Aria: Connector created"<<endl;
    
    // Connect to Robot or Simulator
    if (!MRconnector->connectRobot()){
        cout<<"Aria_Task: Could not connect!"<<endl;
        ArLog::log(ArLog::Terse, "Error, could not connect to robot.");
        Aria::logOptions();
        Aria::exit(1);
        return false;
    }
    else
        cout<<"Aria_Task: Robot connected"<<endl;
    
    // Open new thread for processing cycle
    MRrobot->runAsync(false);
    
    cout<<"Aria_Task: Thread started"<<endl;
    
    //MRrobot->lock();
    //MRrobot->com2Bytes(116, 4, 1);
    //MRrobot->unlock();
    
    return true;
}
void Task::updateHook()
{
    TaskBase::updateHook();
    	
    base::MotionCommand2D MRmotion;
    bool MRdoResetOdometry = 0;
    
    AriaTypes::commands::DevicePower MRdeviceOnOff;
    AriaTypes::commands::DirectCommand2Byte MRdirectCommand;
    
    // Process Motion Commands
    if (_transrot_vel.read(MRmotion) != RTT::NoData){
    
        //cout<<"Aria_Task: Command received"<<endl;
        cout<<"Aria_Task: TranslVel "<<MRmotion.translation<<" m/s, RotVel "<<MRmotion.rotation<<" rad/s"<<endl;
        
        MRrobot->lock();
        //cout<<"Aria_Task: Thread locked"<<endl;
        
        MRrobot->setVel(MRmotion.translation * 1000);
        MRrobot->setRotVel(MRmotion.rotation * 180 / M_PI);
        
        MRrobot->unlock();
        //cout<<"Aria_Task: Thread unlocked"<<endl;
    }
    
    /*** Replaced by Operation
    
    // Process Reset of Odometer
    if(_reset_odometry.read(MRdoResetOdometry) != RTT::NoData){
    
        if( MRdoResetOdometry ){
            cout<<"Aria_Task: Resetting Odometer"<<endl;
        
            MRrobot->lock();
            MRrobot->resetTripOdometer();
            MRrobot->unlock();
        }
    }
    
    // Process De-/Activation of Power-Ports
    if(_device_power.read(MRdeviceOnOff) != RTT::NoData){
    
    	cout<<"Aria_Task: Setting Port "<<int(MRdeviceOnOff.portnr)<<" to "<<MRdeviceOnOff.onoff<<endl;
    	
    	// Send command #116, parameter: port-number, onoff (1=on, 0=off)
    	MRrobot->lock();
	MRrobot->com2Bytes(116, MRdeviceOnOff.portnr, MRdeviceOnOff.onoff);
	MRrobot->unlock();
    }
    
    // Process direct commands to microcontroller
    if(_direct_command.read(MRdirectCommand) != RTT::NoData){
    
    	MRrobot->lock();
	MRrobot->com2Bytes(MRdirectCommand.cmdnr, MRdirectCommand.highbyte, MRdirectCommand.lowbyte);
	MRrobot->unlock();
    }
    
    */
    
    // Fetch Motion- and Odometer-Data from Robot, as well as miscellaneous Data
    base::samples::RigidBodyState MRpose;
    AriaTypes::samples::Velocity MRvel;
    AriaTypes::samples::Velocity2 MRvel2;
    AriaTypes::samples::BatteryLevel MRbatteryLevel;
    AriaTypes::samples::Temperature MRtemperature;
    AriaTypes::samples::CompassHeading MRcompass;
    
    AriaTypes::samples::Odometer MRodom;
    AriaTypes::samples::Encoder MRenc;
    
    AriaTypes::samples::Bumpers MRbumpers;
    
    
    MRrobot->lock();
    
    // Position
    MRpose.position = Eigen::Vector3d(MRrobot->getX() / 1000, MRrobot->getY() / 1000, 0); // in meters
    MRpose.orientation = Eigen::AngleAxis<double>(MRrobot->getTh() * M_PI/180, Eigen::Vector3d::UnitZ()); // rad
    
    MRpose.velocity = Eigen::Vector3d(MRrobot->getVel(), 0, 0); // m/s
    MRpose.angular_velocity = Eigen::Vector3d(MRrobot->getRotVel() * M_PI/180, 0, 0); // rad/s
    
    //cout<<"Aria_Task: Theta: "<<MRrobot->getTh()<<"°"<<endl;
    //cout<<"Aria_Task: Yaw "<<MRpose.getYaw()<<", Pitch "<<MRpose.getPitch()<<", Roll "<<MRpose.getRoll()<<endl;
    
    // Velocity
    MRvel.time = base::Time::now();
    MRvel.velTransRot.translation = MRrobot->getVel(); // in mm/s
    MRvel.velTransRot.rotation = MRrobot->getRotVel(); // in deg/s
    
    // Velocity2 (left, right)
    MRvel2.time = base::Time::now();
    MRvel2.velLeft = MRrobot->getLeftVel(); // in mm/s
    MRvel2.velRight = MRrobot->getRightVel(); // in mm/s
    
    // Battery
    MRbatteryLevel.time = base::Time::now();
    MRbatteryLevel.battery = MRrobot->getStateOfCharge();
    // Temperature
    MRtemperature.time = base::Time::now();
    MRtemperature.temp = MRrobot->getTemperature();
    // Compass
    MRcompass.time = base::Time::now();
    MRcompass.heading = MRrobot->getCompass();
    
    // Odomerty
    MRodom.time = base::Time::now();
    MRodom.odomDistance = MRrobot->getTripOdometerDistance();
    MRodom.odomDegrees = MRrobot->getTripOdometerDegrees();
    
    // Raw Data from left and right Encoders
    MRenc.time = base::Time::now();
    MRrobot->requestEncoderPackets();
    MRenc.encLeft = MRrobot->getLeftEncoder();
    MRenc.encRight = MRrobot->getRightEncoder();
    //cout<<"Aria_Task: Encoder L: "<<MRrobot->getLeftEncoder()<<", R: "<<MRrobot->getRightEncoder()<<endl;
    MRrobot->stopEncoderPackets();
    
    // Bumpers
    // See Aria Documentation and/or ArModes.cpp for an Example how to read out these values.
    MRbumpers.time = base::Time::now();
    MRbumpers.nrFront = MRrobot->getNumFrontBumpers();
    MRbumpers.nrRear = MRrobot->getNumRearBumpers();
    
    MRbumpers.front = ((MRrobot->getStallValue() & 0xff00) >> 8);
    MRbumpers.rear = ((MRrobot->getStallValue() & 0xff));
    
    
    MRrobot->unlock();
    
    bool frbump[MRbumpers.nrFront];
    bool rebump[MRbumpers.nrRear];
    int bit = 0;
    int i = 0;
    
    for(i = 0, bit = 2; i < MRbumpers.nrFront; i++, bit *= 2)
    	frbump[i] = (MRbumpers.front & bit);
    
    for (i = 0, bit = 2; i < MRbumpers.nrRear; i++, bit *= 2)
    	rebump[i] = (MRbumpers.rear & bit);
    
    /*
    //cout<<"Aria_Task: Front Bumpers: ";
    for(int i=0; i<MRbumpers.nrFront; i++)
    {
    	cout<<frbump[i]<<" ";
    }
    cout<<endl;
    
    //cout<<"Aria_Task: Rear Bumpers: ";
    for(int i=0; i<MRbumpers.nrRear; i++)
    {
    	cout<<rebump[i]<<" ";
    }
    cout<<endl;
    */
    
    
    // Distribute Messages
    _robot_pose.write(MRpose);
    _robot_motion.write(MRvel);
    _robot_motion2.write(MRvel2);
    _robot_battery.write(MRbatteryLevel);
    _robot_temp.write(MRtemperature);
    _robot_compass.write(MRcompass);
    _robot_odometer.write(MRodom);
    _robot_encoder.write(MRenc);
    _robot_bumpers.write(MRbumpers);
    
}
// void Task::errorHook()
// {
//     TaskBase::errorHook();
// }
void Task::stopHook()
{
    
    TaskBase::stopHook();
    
    MRrobot->stopRunning();
    MRrobot->waitForRunExit();
    
    // Stop Aria background threads
    Aria::shutdown();
    
    delete MRconnector;
    MRconnector = 0;
    
    delete MRrobot;
    MRrobot = 0;
    
    cout<<"Aria_Task: Stopped"<<endl;
}
void Task::cleanupHook()
{
    TaskBase::cleanupHook();
    
    // Cleanup and exit Aria
    Aria::exit(0);
    
    delete MRparser;
    MRparser = 0;
    
    cout<<"Aria_Task: Exit"<<endl;
}

// Operation Methods

// Set the translational and rotational Velocities
void Task::transrotVel(::base::MotionCommand2D const & velocities)
{
	cout<<"Aria_Task: TranslVel "<<velocities.translation<<" m/s, RotVel "<<velocities.rotation<<" rad/s"<<endl;
        
        MRrobot->lock();
        MRrobot->setVel(velocities.translation * 1000);
        MRrobot->setRotVel(velocities.rotation * 180 / M_PI);
        MRrobot->unlock();
}

void Task::transrotVel2(double translational, double rotational)
{
	cout<<"Aria_Task: TranslVel "<<translational<<" m/s, RotVel "<<rotational<<" rad/s"<<endl;
	
	MRrobot->lock();
        MRrobot->setVel(translational * 1000);
        MRrobot->setRotVel(rotational * 180 / M_PI);
        MRrobot->unlock();
}

// Set Velocities for left and right Wheels
void Task::lrVel(double left, double right)
{
	cout<<"Aria_Task: Velocity L: "<<left<<" m/s, R: "<<right<<" m/s"<<endl;
	
	MRrobot->lock();
	MRrobot->setVel2(left*1000, right*1000);
	MRrobot->unlock();
}

// Turn on/off the PDB
void Task::controlPDB(boost::int32_t portNr, bool onoff)
{
	cout<<"Aria_Task: Turning Port "<<portNr<<" ";
	onoff ? cout<<"ON":cout<<"OFF";
	cout<<endl;
    	
    	// Send command #116, parameter: port-number, onoff (1=on, 0=off)
    	MRrobot->lock();
	MRrobot->com2Bytes(116, portNr, onoff);
	MRrobot->unlock();
}

// Send a direct serial Command to Robot
void Task::directCommand(::AriaTypes::commands::DirectCommand2Byte const & MRcmd2byte)
{
	cout<<"Aria_Task: Direct Command "<<MRcmd2byte.cmdnr<<" with HB: "<<MRcmd2byte.highbyte<<" LB: "<<MRcmd2byte.lowbyte<<endl;
	
	MRrobot->lock();
	MRrobot->com2Bytes(MRcmd2byte.cmdnr, MRcmd2byte.highbyte, MRcmd2byte.lowbyte);
	MRrobot->unlock();
}

// Reset the Odometer
void Task::resetOdometer()
{
    cout<<"Aria_Task: Resetting Odometer"<<endl;

    MRrobot->lock();
    MRrobot->resetTripOdometer();
    MRrobot->unlock();
}

