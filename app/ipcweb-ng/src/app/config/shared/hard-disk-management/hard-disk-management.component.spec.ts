import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { HardDiskManagementComponent } from './hard-disk-management.component';

describe('HardDiskManagementComponent', () => {
  let component: HardDiskManagementComponent;
  let fixture: ComponentFixture<HardDiskManagementComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ HardDiskManagementComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(HardDiskManagementComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
