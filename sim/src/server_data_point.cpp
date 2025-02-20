#include <ItcLogger/Logger.hpp>
#include <server_data_point.hpp>

namespace Nos3
{
    extern ItcLogger::Logger *sim_logger;

    ServerDataPoint::ServerDataPoint(double count)
    {
        sim_logger->trace("ServerDataPoint::ServerDataPoint:  Defined Constructor executed");

        /* Do calculations based on provided data - also preparing like ADC data to checkout is obvious */
        _42_parsing = false;
        _server_data_is_valid = true;
        _server_data[0] = (((count * 1) / 32767.0) - 32768.0);
        _server_data[1] = (((count * 2) / 32767.0) - 32768.0);
        _server_data[2] = (((count * 3) / 32767.0) - 32768.0);
    }

    ServerDataPoint::ServerDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp) : _dp(*dp), _sc(spacecraft)
    {
        sim_logger->trace("ServerDataPoint::ServerDataPoint:  42 Constructor executed");

        /* Initialize data */
        _42_parsing = true;
        _server_data_is_valid = false;
        _server_data[0] = _server_data[1] = _server_data[2] = 0.0;
    }
    
    void ServerDataPoint::do_parsing(void) const
    {
        if (_42_parsing)
        {
            try {
                /*
                ** Declare 42 telemetry string prefix
                ** 42 variables defined in `42/Include/42types.h`
                ** 42 data stream defined in `42/Source/IPC/SimWriteToSocket.c`
                */
                std::string key;
                key.append("SC[").append(std::to_string(_sc)).append("].svb"); // SC[N].svb

                /* Parse 42 telemetry */
                std::string values = _dp.get_value_for_key(key);

                std::vector<double> data;
                data.reserve(3);
                parse_double_vector(values, data);

                _server_data[0] = data[0];
                _server_data[1] = data[1];
                _server_data[2] = data[2];

                /* Mark data as valid */
                _server_data_is_valid = true;

                _not_parsed = false;

                /* Debug print */
                sim_logger->trace("ServerDataPoint::ServerDataPoint:  Parsed svb = %f %f %f", _server_data[0], _server_data[1], _server_data[2]);
            } catch (const std::exception &e) {
                sim_logger->error("ServerDataPoint::ServerDataPoint:  Error parsing svb.  Error=%s", e.what());
            }
        }
    }

    /* Used for printing a representation of the data point */
    std::string ServerDataPoint::to_string(void) const
    {
        sim_logger->trace("ServerDataPoint::to_string:  Executed");
        
        std::stringstream ss;

        ss << std::fixed << std::setfill(' ');
        ss << "Server Data Point:   Valid: ";
        ss << (_server_data_is_valid ? "Valid" : "INVALID");
        ss << std::setprecision(std::numeric_limits<double>::digits10); /* Full double precision */
        ss << " Server Data: "
           << _server_data[0]
           << " "
           << _server_data[1]
           << " "
           << _server_data[2];

        return ss.str();
    }
} /* namespace Nos3 */
