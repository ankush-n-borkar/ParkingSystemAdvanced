import { Car, Clock, MapPin, ArrowLeft } from 'lucide-react';
import './Header.css';

const Header = ({ data, lastUpdated, onBackToHome }) => {
  const formatTime = (date) => {
    return date.toLocaleTimeString('en-IN', { 
      hour: '2-digit', 
      minute: '2-digit',
      second: '2-digit'
    });
  };

  return (
    <header className="header">
      <div className="header-content">
        {onBackToHome && (
          <button className="home-btn" onClick={onBackToHome}>
            <ArrowLeft size={20} />
            Home
          </button>
        )}
        
        <div className="header-title">
          <Car size={32} className="header-icon" />
          <div>
            <h1>Multi-Floor Parking Management System</h1>
            <p className="subtitle">Skyline Apartments</p>
          </div>
        </div>

        <div className="header-stats">
          <div className="stat-box">
            <MapPin size={20} />
            <div>
              <div className="stat-value">{data.total_slots}</div>
              <div className="stat-label">Total Slots</div>
            </div>
          </div>

          <div className="stat-box occupied">
            <Car size={20} />
            <div>
              <div className="stat-value">{data.total_occupied}</div>
              <div className="stat-label">Occupied</div>
            </div>
          </div>

          <div className="stat-box available">
            <Car size={20} />
            <div>
              <div className="stat-value">{data.total_slots - data.total_occupied}</div>
              <div className="stat-label">Available</div>
            </div>
          </div>

          <div className="stat-box">
            <Clock size={20} />
            <div>
              <div className="stat-value">{Math.round(data.occupancy_rate)}%</div>
              <div className="stat-label">Occupancy</div>
            </div>
          </div>
        </div>

        <div className="last-updated">
          Last updated: {formatTime(lastUpdated)}
        </div>
      </div>
    </header>
  );
};

export default Header;
