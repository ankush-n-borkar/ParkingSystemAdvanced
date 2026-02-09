import { useState } from 'react';
import { X, Car } from 'lucide-react';
import './Modal.css';

const ParkModal = ({ isOpen, onClose, onSubmit, slot }) => {
  const [vehicleNumber, setVehicleNumber] = useState('');
  const [vehicleType, setVehicleType] = useState(1);

  if (!isOpen || !slot) return null;

  const handleSubmit = (e) => {
    e.preventDefault();
    
    if (!vehicleNumber.trim()) {
      alert('Please enter a vehicle number');
      return;
    }
    
    // CRITICAL: Include slot_id
    const parkingData = {
      slot_id: slot.id,
      vehicle_number: vehicleNumber.trim(),
      vehicle_type: vehicleType
    };
    
    console.log('ParkModal submitting:', parkingData); // Debug log
    
    onSubmit(parkingData);
    
    setVehicleNumber('');
    setVehicleType(1);
  };

  const handleClose = () => {
    setVehicleNumber('');
    setVehicleType(1);
    onClose();
  };

  return (
    <div className="modal-overlay" onClick={handleClose}>
      <div className="modal-content" onClick={(e) => e.stopPropagation()}>
        <button className="modal-close" onClick={handleClose}>
          <X size={20} />
        </button>

        <div className="modal-header">
          <Car size={32} className="modal-icon" />
          <h2>Park Vehicle</h2>
          <p>Enter vehicle details to proceed</p>
        </div>

        <div className="slot-info-card">
          <div className="slot-info-label">PARKING SLOT</div>
          <div className="slot-info-id">{slot.id}</div>
          <div className="slot-info-details">
            <span className="slot-badge">{slot.type}</span>
            <span className="slot-badge">Floor {slot.id.charAt(1)}</span>
          </div>
        </div>

        <form onSubmit={handleSubmit}>
          <div className="form-group">
            <label>
              <Car size={16} />
              Vehicle Number
            </label>
            <input
              type="text"
              placeholder="E.G., KA01AB1234"
              value={vehicleNumber}
              onChange={(e) => setVehicleNumber(e.target.value.toUpperCase())}
              autoFocus
              required
            />
          </div>

          <div className="form-group">
            <label>
              <Car size={16} />
              Vehicle Type
            </label>
            <div className="vehicle-types">
              <button
                type="button"
                className={`vehicle-type-btn ${vehicleType === 0 ? 'active' : ''}`}
                onClick={() => setVehicleType(0)}
              >
                <span className="vehicle-icon">🏍️</span>
                <div>
                  <div className="vehicle-type-name">Compact</div>
                  <div className="vehicle-type-desc">Two-wheelers, small cars</div>
                </div>
              </button>

              <button
                type="button"
                className={`vehicle-type-btn ${vehicleType === 1 ? 'active' : ''}`}
                onClick={() => setVehicleType(1)}
              >
                <span className="vehicle-icon">🚗</span>
                <div>
                  <div className="vehicle-type-name">Standard</div>
                  <div className="vehicle-type-desc">Sedans, hatchbacks</div>
                </div>
              </button>

              <button
                type="button"
                className={`vehicle-type-btn ${vehicleType === 2 ? 'active' : ''}`}
                onClick={() => setVehicleType(2)}
              >
                <span className="vehicle-icon">🚙</span>
                <div>
                  <div className="vehicle-type-name">Large</div>
                  <div className="vehicle-type-desc">SUVs, vans</div>
                </div>
              </button>
            </div>
          </div>

          <div className="modal-actions">
            <button type="button" className="btn-secondary" onClick={handleClose}>
              Cancel
            </button>
            <button type="submit" className="btn-primary">
              <Car size={18} />
              Confirm Parking
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};

export default ParkModal;
