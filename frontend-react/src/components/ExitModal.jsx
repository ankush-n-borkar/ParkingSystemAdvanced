import { X, LogOut, AlertTriangle } from 'lucide-react';
import './ExitModal.css';

const ExitModal = ({ isOpen, onClose, onConfirm, slot }) => {
  if (!isOpen || !slot) return null;

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content" onClick={(e) => e.stopPropagation()}>
        <button className="modal-close" onClick={onClose}>
          <X size={20} />
        </button>

        <div className="modal-header">
          <div className="modal-icon">
            <LogOut size={32} />
          </div>
          <h2>Exit Vehicle</h2>
          <p>Confirm vehicle exit from parking</p>
        </div>

        <div className="exit-warning">
          <div className="exit-warning-icon">
            <AlertTriangle size={20} />
          </div>
          <div className="exit-warning-text">
            <p>Are you sure you want to exit this vehicle from the parking lot?</p>
          </div>
        </div>

        <div className="slot-details-card">
          <div className="detail-row">
            <span className="detail-label">Slot ID</span>
            <span className="detail-value">{slot.id}</span>
          </div>
          
          <div className="detail-row">
            <span className="detail-label">Vehicle Number</span>
            <span className="detail-value">{slot.vehicle_number || 'N/A'}</span>
          </div>
          
          <div className="detail-row">
            <span className="detail-label">Slot Type</span>
            <span className="detail-badge">{slot.type}</span>
          </div>
          
          <div className="detail-row">
            <span className="detail-label">Floor</span>
            <span className="detail-value">Floor {slot.id.charAt(1)}</span>
          </div>
        </div>

        <div className="modal-actions">
          <button className="btn-cancel" onClick={onClose}>
            Cancel
          </button>
          <button className="btn-exit" onClick={onConfirm}>
            <LogOut size={18} />
            Confirm Exit
          </button>
        </div>
      </div>
    </div>
  );
};

export default ExitModal;
