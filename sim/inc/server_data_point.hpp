#ifndef NOS3_SERVERDATAPOINT_HPP
#define NOS3_SERVERDATAPOINT_HPP

#include <boost/shared_ptr.hpp>
#include <sim_42data_point.hpp>

namespace Nos3
{
    /* Standard for a data point used transfer data between a data provider and a hardware model */
    class ServerDataPoint : public Sim42DataPoint
    {
    public:
        /* Constructors */
        ServerDataPoint(double count);
        ServerDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        /* Accessors */
        /* Provide the hardware model a way to get the specific data out of the data point */
        std::string to_string(void) const;
        double      get_server_data_x(void) const {parse_data_point(); return _server_data[0];}
        double      get_server_data_y(void) const {parse_data_point(); return _server_data[1];}
        double      get_server_data_z(void) const {parse_data_point(); return _server_data[2];}
        bool        is_server_data_valid(void) const {parse_data_point(); return _server_data_is_valid;}
    
    private:
        /* Disallow these */
        ServerDataPoint(void) {};
        ServerDataPoint(const ServerDataPoint& sdp) : Sim42DataPoint(sdp) {};
        ~ServerDataPoint(void) {};

        // Private mutators
        inline void parse_data_point(void) const {if (_not_parsed) do_parsing();}
        void do_parsing(void) const;

        mutable Sim42DataPoint _dp;
        int16_t _sc;
        // mutable below so parsing can be on demand:
        mutable bool _42_parsing;
        mutable bool _not_parsed;
        /* Specific data you need to get from the data provider to the hardware model */
        /* You only get to this data through the accessors above */
        mutable bool   _server_data_is_valid;
        mutable double _server_data[3];
    };
}

#endif
