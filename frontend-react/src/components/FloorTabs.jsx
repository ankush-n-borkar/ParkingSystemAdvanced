import { AlertCircle } from 'lucide-react';
import './FloorTabs.css';

const FloorTabs = ({ floors, currentFloor, onFloorChange, mode }) => {
  return (
    <div className="floor-tabs">
      {floors.map((floor, index) => {
        const available = floor.total_slots - floor.occupied_slots;
        const isFull = available === 0;
        const isCurrentFull = floors[currentFloor].total_slots - floors[currentFloor].occupied_slots === 0;
        
        return (
          <button
            key={index}
            className={`floor-tab ${currentFloor === index ? 'active' : ''} ${isFull ? 'full' : ''} ${isCurrentFull && index !== currentFloor && !isFull ? 'blink' : ''}`}
            onClick={() => onFloorChange(index)}
          >
            <div className="floor-info">
              <div className="floor-number">Floor {floor.floor_number}</div>
              <div className="floor-name">{floor.name}</div>
            </div>
            <div className="floor-stats">
              <div className="occupancy">{floor.occupied_slots}/{floor.total_slots} occupied</div>
              {isFull && mode === 'park' && (
                <div className="full-badge">
                  <AlertCircle size={14} />
                  FULL
                </div>
              )}
              {isCurrentFull && index !== currentFloor && !isFull && mode === 'park' && (
                <div className="available-badge">
                  ✓ Available
                </div>
              )}
            </div>
          </button>
        );
      })}
    </div>
  );
};

export default FloorTabs;
