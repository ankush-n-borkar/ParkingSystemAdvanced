import { Car, CheckCircle } from 'lucide-react';
import './ParkingGrid.css';

function ParkingGrid({ floor, onSlotClick }) {
  return (
    <div className="parking-grid-container">
      <div className="floor-info">
        <h2>{floor.name}</h2>
        <div className="floor-stats">
          <span>Occupancy: {floor.occupancy_rate.toFixed(1)}%</span>
          <span>{floor.occupied_slots}/{floor.total_slots} occupied</span>
        </div>
      </div>

      <div className="parking-grid">
        {floor.slots.map((slot) => (
          <ParkingSlot
            key={slot.id}
            slot={slot}
            onClick={() => onSlotClick(slot)}
          />
        ))}
      </div>

      <Legend />
    </div>
  );
}

function ParkingSlot({ slot, onClick }) {
  const getSlotClass = () => {
    let classes = ['parking-slot', slot.type];
    if (slot.is_occupied) classes.push('occupied');
    else classes.push('free');
    return classes.join(' ');
  };

  return (
    <div className={getSlotClass()} onClick={onClick}>
      <div className="slot-header">
        <span className="slot-id">{slot.id}</span>
        <span className={`slot-status ${slot.is_occupied ? 'occupied' : 'free'}`}>
          {slot.is_occupied ? <Car size={16} /> : <CheckCircle size={16} />}
        </span>
      </div>
      
      <div className="slot-type">{capitalizeFirst(slot.type)}</div>
      
      {slot.is_occupied && (
        <div className="vehicle-info">
          <Car size={14} />
          <span>{slot.vehicle_number}</span>
        </div>
      )}
      
      <div className="usage-count">Used: {slot.usage_count}×</div>
    </div>
  );
}

function Legend() {
  return (
    <div className="legend">
      <div className="legend-item">
        <div className="legend-color compact"></div>
        <span>Compact</span>
      </div>
      <div className="legend-item">
        <div className="legend-color standard"></div>
        <span>Standard</span>
      </div>
      <div className="legend-item">
        <div className="legend-color large"></div>
        <span>Large</span>
      </div>
      <div className="legend-item">
        <div className="legend-color disabled"></div>
        <span>Disabled</span>
      </div>
    </div>
  );
}

function capitalizeFirst(str) {
  return str.charAt(0).toUpperCase() + str.slice(1);
}

export default ParkingGrid;
