import { useState, useMemo } from 'react';
import { Car, LogOut, MapPin, ArrowRight } from 'lucide-react';
import './LandingPage.css';


const LandingPage = ({ onSelectAction, parkingData }) => {
  const [selectedAction, setSelectedAction] = useState(null);

  // ✅ FIX: Use useMemo to recalculate when parkingData changes
  const actions = useMemo(() => [
    {
      id: 'park',
      icon: <Car size={48} />,
      title: 'Park Vehicle',
      description: 'Find and book the nearest available parking slot',
      color: '#10b981',
      available: parkingData?.total_slots - parkingData?.total_occupied || 0
    },
    {
      id: 'exit',
      icon: <LogOut size={48} />,
      title: 'Exit Parking',
      description: 'Free your parking slot and exit the facility',
      color: '#ef4444',
      occupied: parkingData?.total_occupied || 0
    },
    {
      id: 'visualize',
      icon: <MapPin size={48} />,
      title: 'View Layout',
      description: 'Interactive map with real-time slot availability',
      color: '#6366f1',
      floors: parkingData?.total_floors || 0
    }
  ], [parkingData?.total_slots, parkingData?.total_occupied, parkingData?.total_floors]); // ✅ Dependencies


  const handleSelect = (actionId) => {
    setSelectedAction(actionId);
    setTimeout(() => onSelectAction(actionId), 300);
  };


  return (
    <div className="landing-page">
      <div className="landing-header">
        <div className="building-icon">🏢</div>
        <h1>Navami Apartments</h1>
        <p className="subtitle">Multi-Level Smart Parking System</p>
      </div>


      <div className="parking-stats">
        <div className="stat-card">
          <div className="stat-value">{parkingData?.total_slots || 0}</div>
          <div className="stat-label">Total Slots</div>
        </div>
        <div className="stat-card available">
          <div className="stat-value">{(parkingData?.total_slots || 0) - (parkingData?.total_occupied || 0)}</div>
          <div className="stat-label">Available</div>
        </div>
        <div className="stat-card occupied">
          <div className="stat-value">{parkingData?.total_occupied || 0}</div>
          <div className="stat-label">Occupied</div>
        </div>
      </div>


      <div className="action-selector">
        <h2>What would you like to do?</h2>
        <div className="action-cards">
          {actions.map((action) => (
            <div
              key={action.id}
              className={`action-card ${selectedAction === action.id ? 'selected' : ''}`}
              onClick={() => handleSelect(action.id)}
              style={{ '--action-color': action.color }}
            >
              <div className="action-icon">{action.icon}</div>
              <h3>{action.title}</h3>
              <p>{action.description}</p>
              
              {action.available !== undefined && (
                <div className="action-meta">
                  <span className="badge success">{action.available} slots free</span>
                </div>
              )}
              {action.occupied !== undefined && (
                <div className="action-meta">
                  <span className="badge danger">{action.occupied} vehicles parked</span>
                </div>
              )}
              {action.floors !== undefined && (
                <div className="action-meta">
                  <span className="badge primary">{action.floors} floors</span>
                </div>
              )}
              
              <div className="action-arrow">
                <ArrowRight size={24} />
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};


export default LandingPage;
