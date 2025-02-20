#include <server_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(ServerDataProvider,"SERVER_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    ServerDataProvider::ServerDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("ServerDataProvider::ServerDataProvider:  Constructor executed");
        _request_count = 0;
    }

    boost::shared_ptr<SimIDataPoint> ServerDataProvider::get_data_point(void) const
    {
        sim_logger->trace("ServerDataProvider::get_data_point:  Executed");

        /* Prepare the provider data */
        _request_count++;

        /* Request a data point */
        SimIDataPoint *dp = new ServerDataPoint(_request_count);

        /* Return the data point */
        return boost::shared_ptr<SimIDataPoint>(dp);
    }
}
