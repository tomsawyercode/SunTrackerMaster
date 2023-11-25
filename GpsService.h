//struct time { int year, month, day, hh, mm, ss;}; //definided en TrackMaster.h



enum gpsOrigin { MASTER, SLAVE} ;

class GpsService
{
public:


//GpsService(); came with compilation problems like: request from member ...
GpsService(gpsOrigin );
int getOrigin(){return _origin;};//
void retrieve();
//void printVars(size_t (*fpr)(const char* format,...));
void printVars( );


bool getStatus(int & v1,int & v2,int & v3);
void getSats(int & v1,int  & v2);// inViev,InUse
bool getLocation(float & longitude,float & latitude,float & elevation);
bool getUtcDateTime(byte * p);

private:
gpsOrigin _origin;

bool updDateTimeFromMaster(byte * p);
bool updDateTimeFromSlave(byte * p);

void printMasterVars();
void printSlaveVars();

bool getSlaveStatus(int & v1,int & v2,int & v3);
bool getSlaveLocation(float & longitude,float & latitude,float & elevation);
bool getSlaveDate(byte * p);
bool getSlaveTime(byte * p);





};
